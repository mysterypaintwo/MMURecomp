// KERNEL32 shims for the recompiled MMU.exe + msvcr90/msvcp90 (stdcall).
//
// Single-threaded Windows semantics. Paths: the guest sees its directory as
// C:\MMU (GetModuleFileName/GetCurrentDirectory); "C:\MMU\x" and relative
// paths map to <game dir>/x with backslashes converted.
#include "recomp/runtime.h"
#include "recomp/shims.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

constexpr uint32_t kErrFileNotFound = 2;
constexpr uint32_t kErrInvalidHandle = 6;
constexpr uint32_t kInvalidHandle = 0xffffffffu;
constexpr uint32_t kStdIn = 0x10, kStdOut = 0x14, kStdErr = 0x18;
constexpr uint32_t kProcessHeap = 0x00ee0000u;
constexpr uint32_t kKernel32Handle = 0x7c800000u;

uint32_t g_last_error = 0;
std::unordered_map<uint32_t, FILE*> g_files;
uint32_t g_next_handle = 0x100;
uint32_t g_tls[128];
bool g_tls_used[128];
uint32_t g_cmdline_a = 0, g_cmdline_w = 0, g_env_a = 0, g_env_w = 0;
const auto g_start = std::chrono::steady_clock::now();

void set_error(uint32_t e) {
    g_last_error = e;
    rt_wr32(RT_FS_BASE + 0x34, e);
}

std::string wide_to_utf8(uint32_t p, int len = -1) {
    std::string out;
    for (int i = 0; len < 0 || i < len; ++i) {
        uint16_t ch;
        memcpy(&ch, rt_mem + p + 2u * (uint32_t)i, 2);
        if (len < 0 && ch == 0) break;
        out.push_back(ch < 0x100 ? (char)ch : '?');
    }
    return out;
}

uint32_t guest_wide(const std::string& s) {
    const uint32_t p = rt_malloc((uint32_t)(s.size() + 1) * 2u);
    for (size_t i = 0; i <= s.size(); ++i) {
        uint16_t ch = i < s.size() ? (uint8_t)s[i] : 0;
        memcpy(rt_mem + p + 2 * i, &ch, 2);
    }
    return p;
}

uint32_t module_base_by_name(const std::string& raw) {
    std::string name;
    for (char ch : raw) name.push_back((char)tolower((unsigned char)ch));
    const size_t slash = name.find_last_of("\\/");
    if (slash != std::string::npos) name = name.substr(slash + 1);
    if (name.find('.') == std::string::npos) name += ".dll";
    for (unsigned i = 0; i < rt_module_count; ++i) {
        std::string f = rt_modules[i].file;
        for (char& ch : f) ch = (char)tolower((unsigned char)ch);
        if (f == name) return rt_modules[i].base;
    }
    if (name == "kernel32.dll") return kKernel32Handle;
    return 0;
}

// Export lookup in a loaded (lifted) module image.
uint32_t find_export(uint32_t base, const char* want, uint32_t ordinal) {
    const uint32_t pe = base + rt_rd32(base + 0x3c);
    const uint32_t dir = rt_rd32(pe + 24 + 96);
    if (!dir) return 0;
    const uint32_t e = base + dir;
    const uint32_t ordbase = rt_rd32(e + 16), nfun = rt_rd32(e + 20), nname = rt_rd32(e + 24);
    const uint32_t afun = base + rt_rd32(e + 28), aname = base + rt_rd32(e + 32), aord = base + rt_rd32(e + 36);
    if (!want) {
        const uint32_t k = ordinal - ordbase;
        return k < nfun ? base + rt_rd32(afun + 4 * k) : 0;
    }
    for (uint32_t k = 0; k < nname; ++k) {
        if (strcmp(rt_str(base + rt_rd32(aname + 4 * k)), want) == 0) {
            uint16_t ord;
            memcpy(&ord, rt_mem + aord + 2 * k, 2);
            return base + rt_rd32(afun + 4u * ord);
        }
    }
    return 0;
}

FILE* file_of(uint32_t h) {
    if (h == kStdOut) return stdout;
    if (h == kStdErr) return stderr;
    if (h == kStdIn) return stdin;
    auto it = g_files.find(h);
    return it == g_files.end() ? nullptr : it->second;
}

// cp1252 0x80..0x9f -> UTF-16; other bytes map to themselves.
const uint16_t kCp1252High[32] = {
    0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0160,
    0x2039, 0x0152, 0x008d, 0x017d, 0x008f, 0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022,
    0x2013, 0x2014, 0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x017e, 0x0178};
uint16_t cp1252_to_wide(uint8_t b) { return (b >= 0x80 && b < 0xa0) ? kCp1252High[b - 0x80] : b; }
int wide_to_cp1252(uint16_t w) {
    if (w < 0x80 || (w >= 0xa0 && w < 0x100)) return w;
    for (int i = 0; i < 32; ++i)
        if (kCp1252High[i] == w) return 0x80 + i;
    return -1;
}

// GetStringType C1 classification for cp1252/Latin-1 code points.
uint16_t ctype1(uint16_t w) {
    uint16_t t = 0;
    if (w < 0x80) {
        const int ch = w;
        if (isupper(ch)) t |= 0x001;
        if (islower(ch)) t |= 0x002;
        if (isdigit(ch)) t |= 0x004;
        if (isspace(ch)) t |= 0x008;
        if (ispunct(ch)) t |= 0x010;
        if (iscntrl(ch)) t |= 0x020;
        if (ch == ' ' || ch == '\t') t |= 0x040;
        if (isxdigit(ch)) t |= 0x080;
        if (isalpha(ch)) t |= 0x100;
        if (ch != 0) t |= 0x200; // C1_DEFINED
        return t;
    }
    t |= 0x200;
    if ((w >= 0xc0 && w <= 0xde && w != 0xd7) || w == 0x160 || w == 0x152 || w == 0x17d || w == 0x178) t |= 0x101;
    else if ((w >= 0xdf && w <= 0xff && w != 0xf7) || w == 0x161 || w == 0x153 || w == 0x17e || w == 0x192 || w == 0xaa || w == 0xba || w == 0xb5) t |= 0x102;
    else if (w == 0xa0) t |= 0x048;
    else if (w < 0xa0) t |= 0x020;
    else t |= 0x010;
    return t;
}

void write_filetime(uint32_t p, uint64_t ft) {
    rt_wr32(p, (uint32_t)ft);
    rt_wr32(p + 4, (uint32_t)(ft >> 32));
}
uint64_t now_filetime() {
    using namespace std::chrono;
    const uint64_t us = (uint64_t)duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return us * 10u + 116444736000000000ull;
}

} // namespace

extern "C" uint32_t rt_find_export(const char* module, const char* name) {
    const uint32_t base = module_base_by_name(module);
    return base && base != kKernel32Handle ? find_export(base, name, 0) : 0;
}

std::string rt_host_path(const char* win) {
    std::string p = win ? win : "";
    for (char& ch : p) if (ch == '\\') ch = '/';
    const char* root = "C:/MMU";
    if (p.size() >= 6 && strncasecmp(p.c_str(), root, 6) == 0) p = p.substr(6);
    while (!p.empty() && p[0] == '/') p.erase(0, 1);
    if (p.size() >= 2 && p[1] == ':') p = p.substr(2); // other drives: best effort
    return std::string(rt_game_dir()) + "/" + p;
}

#define K32(name) extern "C" void imp_KERNEL32__##name(Cpu* c)
#define ARG(n) rt_arg(c, n)
#define RET(v, n) do { c->eax = (uint32_t)(v); RT_RET_STDCALL(c, n); return; } while (0)

// ---- process / modules
K32(GetModuleHandleA) { RET(ARG(0) ? module_base_by_name(rt_str(ARG(0))) : rt_modules[0].base, 1); }
K32(GetModuleHandleW) { RET(ARG(0) ? module_base_by_name(wide_to_utf8(ARG(0))) : rt_modules[0].base, 1); }
K32(LoadLibraryA) {
    const uint32_t base = module_base_by_name(rt_str(ARG(0)));
    if (getenv("RECOMP_TRACE_LOAD")) fprintf(stderr, "LoadLibraryA(%s) -> %x\n", rt_str(ARG(0)), base);
    if (!base) set_error(126);
    RET(base, 1);
}
K32(LoadLibraryW) { RET(module_base_by_name(wide_to_utf8(ARG(0))), 1); }
K32(FreeLibrary) { RET(1, 1); }
K32(DisableThreadLibraryCalls) { RET(1, 1); }
K32(GetProcAddress) {
    const uint32_t mod = ARG(0), name = ARG(1);
    uint32_t r = 0;
    if (mod == kKernel32Handle) {
        r = name > 0xffff ? rt_shim_address(rt_str(name)) : 0;
    } else if (mod) {
        r = name > 0xffff ? find_export(mod, rt_str(name), 0) : find_export(mod, nullptr, name);
    }
    if (!r) set_error(127);
    if (getenv("RECOMP_TRACE_LOAD"))
        fprintf(stderr, "GetProcAddress(%x, %s) -> %x\n", mod, name > 0xffff ? rt_str(name) : "#ord", r);
    RET(r, 2);
}
K32(GetModuleFileNameA) {
    std::string path = "C:\\MMU\\";
    path += ARG(0) ? "" : rt_modules[0].file;
    for (unsigned i = 0; ARG(0) && i < rt_module_count; ++i)
        if (rt_modules[i].base == ARG(0)) path += rt_modules[i].file;
    const uint32_t n = ARG(2);
    const uint32_t len = (uint32_t)path.size() < n ? (uint32_t)path.size() : n - 1;
    memcpy(rt_mem + ARG(1), path.c_str(), len);
    rt_mem[ARG(1) + len] = 0;
    RET(len, 3);
}
K32(GetModuleFileNameW) {
    std::string path = std::string("C:\\MMU\\") + rt_modules[0].file;
    const uint32_t n = ARG(2);
    uint32_t len = (uint32_t)path.size() < n ? (uint32_t)path.size() : n - 1;
    for (uint32_t i = 0; i <= len; ++i) {
        uint16_t ch = i < len ? (uint8_t)path[i] : 0;
        memcpy(rt_mem + ARG(1) + 2 * i, &ch, 2);
    }
    RET(len, 3);
}
K32(GetCommandLineA) {
    if (!g_cmdline_a) g_cmdline_a = rt_strdup_guest("\"C:\\MMU\\MMU.exe\"");
    RET(g_cmdline_a, 0);
}
K32(GetCommandLineW) {
    if (!g_cmdline_w) g_cmdline_w = guest_wide("\"C:\\MMU\\MMU.exe\"");
    RET(g_cmdline_w, 0);
}
K32(GetStartupInfoA) {
    memset(rt_mem + ARG(0), 0, 68);
    rt_wr32(ARG(0), 68);
    RET(0, 1);
}
K32(GetEnvironmentStrings) {
    if (!g_env_a) g_env_a = rt_malloc(2);
    RET(g_env_a, 0);
}
K32(GetEnvironmentStringsW) {
    if (!g_env_w) g_env_w = rt_malloc(4);
    RET(g_env_w, 0);
}
K32(FreeEnvironmentStringsA) { RET(1, 1); }
K32(FreeEnvironmentStringsW) { RET(1, 1); }
K32(GetEnvironmentVariableA) { set_error(203); RET(0, 3); }
K32(SetEnvironmentVariableA) { RET(1, 2); }
K32(SetEnvironmentVariableW) { RET(1, 2); }
K32(GetCurrentProcess) { RET(0xffffffffu, 0); }
K32(GetCurrentThread) { RET(0xfffffffeu, 0); }
K32(GetCurrentProcessId) { RET(0x1000, 0); }
K32(GetCurrentThreadId) { RET(0x1004, 0); }
K32(ExitProcess) { rt_crumb("ExitProcess(%u)", ARG(0)); fflush(stdout); exit((int)ARG(0)); }
K32(TerminateProcess) { fflush(stdout); exit((int)ARG(1)); }
K32(ExitThread) { fflush(stdout); exit((int)ARG(0)); }
K32(SetPriorityClass) { RET(1, 2); }
K32(IsDebuggerPresent) { RET(0, 0); }
K32(SetUnhandledExceptionFilter) { RET(0, 1); }
K32(UnhandledExceptionFilter) { RET(0, 1); }
K32(SetErrorMode) { RET(0, 1); }
K32(SetHandleCount) { RET(ARG(0), 1); }
K32(SetConsoleCtrlHandler) { RET(1, 2); }
K32(DebugBreak) { rt_fault(0, "DebugBreak"); }
K32(OutputDebugStringA) { fprintf(stderr, "[guest] %s", rt_str(ARG(0))); RET(0, 1); }
K32(RaiseException) {
    fprintf(stderr, "recomp: RaiseException code=0x%08x flags=%u nargs=%u\n", ARG(0), ARG(1), ARG(2));
    rt_fault(0, "RaiseException (guest C++/SEH exception)");
}
K32(RtlUnwind) { rt_fault(0, "RtlUnwind"); }
K32(GetSystemInfo) {
    const uint32_t p = ARG(0);
    memset(rt_mem + p, 0, 36);
    rt_wr32(p + 4, 4096);          // dwPageSize
    rt_wr32(p + 8, 0x00010000u);   // lpMinimumApplicationAddress
    rt_wr32(p + 12, 0x7ffeffffu);  // lpMaximumApplicationAddress
    rt_wr32(p + 16, 1);            // active processor mask
    rt_wr32(p + 20, 1);            // processors
    rt_wr32(p + 24, 586);          // PROCESSOR_INTEL_PENTIUM
    rt_wr32(p + 28, 0x10000);      // allocation granularity
    rt_wr32(p + 32, 6);            // level/revision
    RET(0, 1);
}

// ---- errors
K32(GetLastError) { RET(g_last_error, 0); }
K32(SetLastError) { set_error(ARG(0)); RET(0, 1); }

// ---- heaps and virtual memory
K32(GetProcessHeap) { RET(kProcessHeap, 0); }
K32(HeapCreate) { RET(kProcessHeap + 0x10000, 3); }
K32(HeapDestroy) { RET(1, 1); }
K32(HeapAlloc) { RET(rt_malloc(ARG(2)), 3); }
K32(HeapFree) { rt_free(ARG(2)); RET(1, 3); }
K32(HeapReAlloc) { RET(rt_realloc(ARG(2), ARG(3)), 4); }
K32(HeapSize) { RET(rt_msize(ARG(2)), 3); }
K32(HeapValidate) { RET(1, 3); }
K32(HeapCompact) { RET(0x100000, 2); }
K32(HeapWalk) { set_error(259); RET(0, 2); }
K32(VirtualAlloc) {
    if (ARG(0)) RET(ARG(0), 4); // commit inside an earlier reservation
    const uint32_t raw = rt_malloc(ARG(1) + 0x10000u);
    RET(raw ? (raw + 0xffffu) & ~0xffffu : 0, 4);
}
K32(VirtualFree) { RET(1, 3); }
K32(VirtualProtect) { if (ARG(3)) rt_wr32(ARG(3), 4); RET(1, 4); }
K32(VirtualQuery) {
    const uint32_t p = ARG(1);
    memset(rt_mem + p, 0, 28);
    rt_wr32(p, ARG(0) & ~0xfffu);
    rt_wr32(p + 4, ARG(0) & ~0xffffu);
    rt_wr32(p + 8, 4);
    rt_wr32(p + 12, 0x1000);
    rt_wr32(p + 16, 0x1000);  // MEM_COMMIT
    rt_wr32(p + 20, 4);       // PAGE_READWRITE
    rt_wr32(p + 24, 0x20000); // MEM_PRIVATE
    RET(28, 3);
}

// ---- TLS / sync (single-threaded)
K32(TlsAlloc) {
    for (uint32_t i = 0; i < 128; ++i)
        if (!g_tls_used[i]) { g_tls_used[i] = true; g_tls[i] = 0; RET(i, 0); }
    RET(0xffffffffu, 0);
}
K32(TlsFree) { if (ARG(0) < 128) g_tls_used[ARG(0)] = false; RET(1, 1); }
K32(TlsGetValue) { set_error(0); RET(ARG(0) < 128 ? g_tls[ARG(0)] : 0, 1); }
K32(TlsSetValue) { if (ARG(0) < 128) g_tls[ARG(0)] = ARG(1); RET(1, 2); }
K32(InitializeCriticalSection) { memset(rt_mem + ARG(0), 0, 24); RET(0, 1); }
K32(InitializeCriticalSectionAndSpinCount) { memset(rt_mem + ARG(0), 0, 24); RET(1, 2); }
K32(DeleteCriticalSection) { RET(0, 1); }
K32(EnterCriticalSection) { RET(0, 1); }
K32(LeaveCriticalSection) { RET(0, 1); }
K32(InterlockedIncrement) { uint32_t v = rt_rd32(ARG(0)) + 1; rt_wr32(ARG(0), v); RET(v, 1); }
K32(InterlockedDecrement) { uint32_t v = rt_rd32(ARG(0)) - 1; rt_wr32(ARG(0), v); RET(v, 1); }
K32(InterlockedExchange) { uint32_t o = rt_rd32(ARG(0)); rt_wr32(ARG(0), ARG(1)); RET(o, 2); }
K32(InterlockedExchangeAdd) { uint32_t o = rt_rd32(ARG(0)); rt_wr32(ARG(0), o + ARG(1)); RET(o, 2); }
K32(InterlockedCompareExchange) {
    uint32_t o = rt_rd32(ARG(0));
    if (o == ARG(2)) rt_wr32(ARG(0), ARG(1));
    RET(o, 3);
}
K32(WaitForSingleObject) { RET(0, 2); }
K32(Sleep) { std::this_thread::sleep_for(std::chrono::milliseconds(ARG(0))); RET(0, 1); }
K32(CreateThread) { rt_fault(0, "CreateThread (threads not supported yet)"); }

// ---- time
K32(GetTickCount) {
    using namespace std::chrono;
    RET((uint32_t)duration_cast<milliseconds>(steady_clock::now() - g_start).count(), 0);
}
K32(QueryPerformanceCounter) {
    using namespace std::chrono;
    const uint64_t t = (uint64_t)duration_cast<microseconds>(steady_clock::now() - g_start).count();
    rt_wr32(ARG(0), (uint32_t)t);
    rt_wr32(ARG(0) + 4, (uint32_t)(t >> 32));
    RET(1, 1);
}
K32(GetSystemTimeAsFileTime) { write_filetime(ARG(0), now_filetime()); RET(0, 1); }
K32(GetLocalTime) {
    const std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    uint16_t st[8] = {(uint16_t)(tm.tm_year + 1900), (uint16_t)(tm.tm_mon + 1), (uint16_t)tm.tm_wday,
                      (uint16_t)tm.tm_mday, (uint16_t)tm.tm_hour, (uint16_t)tm.tm_min,
                      (uint16_t)tm.tm_sec, 0};
    memcpy(rt_mem + ARG(0), st, 16);
    RET(0, 1);
}
K32(GetTimeZoneInformation) { memset(rt_mem + ARG(0), 0, 172); RET(0, 1); }
K32(FileTimeToLocalFileTime) { memcpy(rt_mem + ARG(1), rt_mem + ARG(0), 8); RET(1, 2); }
K32(LocalFileTimeToFileTime) { memcpy(rt_mem + ARG(1), rt_mem + ARG(0), 8); RET(1, 2); }
K32(FileTimeToSystemTime) {
    uint64_t ft;
    memcpy(&ft, rt_mem + ARG(0), 8);
    const std::time_t t = (std::time_t)((ft - 116444736000000000ull) / 10000000ull);
    std::tm tm = *std::gmtime(&t);
    uint16_t st[8] = {(uint16_t)(tm.tm_year + 1900), (uint16_t)(tm.tm_mon + 1), (uint16_t)tm.tm_wday,
                      (uint16_t)tm.tm_mday, (uint16_t)tm.tm_hour, (uint16_t)tm.tm_min,
                      (uint16_t)tm.tm_sec, (uint16_t)((ft / 10000ull) % 1000ull)};
    memcpy(rt_mem + ARG(1), st, 16);
    RET(1, 2);
}

// ---- code pages / locale (system code page 1252, LCID 0x409)
K32(GetACP) { RET(1252, 0); }
K32(GetOEMCP) { RET(437, 0); }
K32(GetConsoleCP) { RET(437, 0); }
K32(GetConsoleOutputCP) { RET(437, 0); }
K32(IsValidCodePage) { RET(ARG(0) == 1252 || ARG(0) == 437 || ARG(0) == 65001 || ARG(0) == 20127, 1); }
K32(GetCPInfo) {
    const uint32_t p = ARG(1);
    memset(rt_mem + p, 0, 20);
    rt_wr32(p, 1);       // MaxCharSize
    rt_mem[p + 4] = '?'; // DefaultChar
    RET(1, 2);
}
K32(IsDBCSLeadByteEx) { RET(0, 2); }
K32(GetUserDefaultLCID) { RET(0x409, 0); }
K32(IsValidLocale) { RET(1, 2); }
K32(EnumSystemLocalesA) { RET(1, 2); }
K32(MultiByteToWideChar) {
    const uint32_t src = ARG(2), dst = ARG(4), cap = ARG(5);
    int32_t n = (int32_t)ARG(3);
    if (n < 0) n = (int32_t)strlen(rt_str(src)) + 1;
    if (cap == 0) RET(n, 6);
    const int32_t out = n < (int32_t)cap ? n : (int32_t)cap;
    for (int32_t i = 0; i < out; ++i) {
        const uint16_t w = cp1252_to_wide(rt_mem[src + (uint32_t)i]);
        memcpy(rt_mem + dst + 2u * (uint32_t)i, &w, 2);
    }
    RET(out, 6);
}
K32(WideCharToMultiByte) {
    const uint32_t src = ARG(2), dst = ARG(4), cap = ARG(5), used_default = ARG(7);
    int32_t n = (int32_t)ARG(3);
    if (n < 0) {
        n = 0;
        uint16_t w;
        do { memcpy(&w, rt_mem + src + 2u * (uint32_t)n, 2); ++n; } while (w);
    }
    if (used_default) rt_wr32(used_default, 0);
    if (cap == 0) RET(n, 8);
    const int32_t out = n < (int32_t)cap ? n : (int32_t)cap;
    for (int32_t i = 0; i < out; ++i) {
        uint16_t w;
        memcpy(&w, rt_mem + src + 2u * (uint32_t)i, 2);
        int b = wide_to_cp1252(w);
        if (b < 0) { b = '?'; if (used_default) rt_wr32(used_default, 1); }
        rt_mem[dst + (uint32_t)i] = (uint8_t)b;
    }
    RET(out, 8);
}
K32(GetStringTypeW) {
    const uint32_t src = ARG(1), dst = ARG(3);
    int32_t n = (int32_t)ARG(2);
    for (int32_t i = 0; n < 0 || i < n; ++i) {
        uint16_t w;
        memcpy(&w, rt_mem + src + 2u * (uint32_t)i, 2);
        if (n < 0 && w == 0) break;
        const uint16_t t = ARG(0) == 1 ? ctype1(w) : 0;
        memcpy(rt_mem + dst + 2u * (uint32_t)i, &t, 2);
    }
    RET(1, 4);
}
K32(GetStringTypeA) {
    const uint32_t src = ARG(2), dst = ARG(4);
    int32_t n = (int32_t)ARG(3);
    if (n < 0) n = (int32_t)strlen(rt_str(src));
    for (int32_t i = 0; i < n; ++i) {
        const uint16_t t = ARG(1) == 1 ? ctype1(cp1252_to_wide(rt_mem[src + (uint32_t)i])) : 0;
        memcpy(rt_mem + dst + 2u * (uint32_t)i, &t, 2);
    }
    RET(1, 5);
}
static uint16_t map_case(uint16_t w, uint32_t flags) {
    const bool lower = flags & 0x100, upper = flags & 0x200;
    if (lower) {
        if ((w >= 'A' && w <= 'Z') || (w >= 0xc0 && w <= 0xde && w != 0xd7)) return w + 32;
    } else if (upper) {
        if ((w >= 'a' && w <= 'z') || (w >= 0xe0 && w <= 0xfe && w != 0xf7)) return w - 32;
    }
    return w;
}
K32(LCMapStringW) {
    const uint32_t flags = ARG(1), src = ARG(2), dst = ARG(4), cap = ARG(5);
    int32_t n = (int32_t)ARG(3);
    if (n < 0) { n = 0; uint16_t w; do { memcpy(&w, rt_mem + src + 2u * (uint32_t)n, 2); ++n; } while (w); }
    if (cap == 0) RET(n, 6);
    for (int32_t i = 0; i < n && i < (int32_t)cap; ++i) {
        uint16_t w;
        memcpy(&w, rt_mem + src + 2u * (uint32_t)i, 2);
        w = map_case(w, flags);
        memcpy(rt_mem + dst + 2u * (uint32_t)i, &w, 2);
    }
    RET(n, 6);
}
K32(LCMapStringA) {
    const uint32_t flags = ARG(1), src = ARG(2), dst = ARG(4), cap = ARG(5);
    int32_t n = (int32_t)ARG(3);
    if (n < 0) n = (int32_t)strlen(rt_str(src)) + 1;
    if (cap == 0) RET(n, 6);
    for (int32_t i = 0; i < n && i < (int32_t)cap; ++i) {
        const int b = wide_to_cp1252(map_case(cp1252_to_wide(rt_mem[src + (uint32_t)i]), flags));
        rt_mem[dst + (uint32_t)i] = (uint8_t)(b < 0 ? '?' : b);
    }
    RET(n, 6);
}
K32(CompareStringW) {
    int32_t n1 = (int32_t)ARG(3), n2 = (int32_t)ARG(5);
    const std::string a = wide_to_utf8(ARG(2), n1), b = wide_to_utf8(ARG(4), n2);
    const int r = (ARG(1) & 1) ? strcasecmp(a.c_str(), b.c_str()) : strcmp(a.c_str(), b.c_str());
    RET(r < 0 ? 1 : r > 0 ? 3 : 2, 6);
}
K32(CompareStringA) {
    int32_t n1 = (int32_t)ARG(3), n2 = (int32_t)ARG(5);
    std::string a = n1 < 0 ? rt_str(ARG(2)) : std::string(rt_str(ARG(2)), (size_t)n1);
    std::string b = n2 < 0 ? rt_str(ARG(4)) : std::string(rt_str(ARG(4)), (size_t)n2);
    const int r = (ARG(1) & 1) ? strcasecmp(a.c_str(), b.c_str()) : strcmp(a.c_str(), b.c_str());
    RET(r < 0 ? 1 : r > 0 ? 3 : 2, 6);
}
K32(GetLocaleInfoA) {
    // Only the values msvcr90 asks for while building the "C" locale.
    char buf[16] = "";
    switch (ARG(1) & 0xffff) {
    case 0x1004: strcpy(buf, "1252"); break; // LOCALE_IDEFAULTANSICODEPAGE
    case 0x000b: strcpy(buf, "437"); break;  // LOCALE_IDEFAULTCODEPAGE
    case 0x000e: strcpy(buf, "."); break;    // LOCALE_SDECIMAL
    case 0x000f: strcpy(buf, ","); break;    // LOCALE_STHOUSAND
    default: set_error(87); RET(0, 4);
    }
    const uint32_t n = (uint32_t)strlen(buf) + 1;
    if (ARG(3) == 0) RET(n, 4);
    memcpy(rt_mem + ARG(2), buf, n);
    RET(n, 4);
}
K32(GetLocaleInfoW) { set_error(87); RET(0, 4); }

// ---- files
K32(GetStdHandle) {
    switch (ARG(0)) {
    case 0xfffffff6u: RET(kStdIn, 1);
    case 0xfffffff5u: RET(kStdOut, 1);
    case 0xfffffff4u: RET(kStdErr, 1);
    default: RET(kInvalidHandle, 1);
    }
}
K32(SetStdHandle) { RET(1, 2); }
K32(GetFileType) {
    if (ARG(0) == kStdIn || ARG(0) == kStdOut || ARG(0) == kStdErr) RET(2, 1); // FILE_TYPE_CHAR
    RET(g_files.count(ARG(0)) ? 1 : 0, 1);
}
K32(GetConsoleMode) { set_error(kErrInvalidHandle); RET(0, 2); }
K32(SetConsoleMode) { RET(0, 2); }
K32(CreateFileA) {
    const std::string path = rt_host_path(rt_str(ARG(0)));
    const uint32_t access = ARG(1), disposition = ARG(4);
    const bool write = (access & 0x40000000u) != 0;
    const char* mode = "rb";
    if (write) {
        switch (disposition) {
        case 1: case 2: mode = "wb"; break;        // CREATE_NEW / CREATE_ALWAYS
        case 4: mode = "ab+"; break;               // OPEN_ALWAYS
        default: mode = "rb+"; break;              // OPEN_EXISTING / TRUNCATE
        }
    }
    FILE* f = fopen(path.c_str(), mode);
    if (!f && write && disposition == 4) f = fopen(path.c_str(), "wb+");
    if (!f) {
        set_error(kErrFileNotFound);
        RET(kInvalidHandle, 7);
    }
    if (write && disposition == 4) fseek(f, 0, SEEK_SET);
    // The asset load is ~105k small ReadFile calls. With newlib's small default
    // buffer each became an SD card request on Switch (25 s load); 1 MiB
    // buffers make them memory copies.
    if (!write) setvbuf(f, nullptr, _IOFBF, 1u << 20);
    const uint32_t h = g_next_handle;
    g_next_handle += 4;
    g_files[h] = f;
    set_error(0);
    RET(h, 7);
}
K32(CreateFileW) {
    const std::string name = wide_to_utf8(ARG(0));
    const uint32_t tmp = rt_strdup_guest(name.c_str());
    rt_wr32(c->esp + 4, tmp);
    imp_KERNEL32__CreateFileA(c);
    rt_free(tmp);
}
K32(CloseHandle) {
    auto it = g_files.find(ARG(0));
    if (it != g_files.end()) { fclose(it->second); g_files.erase(it); }
    RET(1, 1);
}
// I/O counters for the status line (recomp/src/log.cpp).
extern "C" { uint64_t rt_io_reads, rt_io_bytes, rt_io_seeks, rt_io_ns; }
extern "C" void rt_load_progress(void);  // imports_sdl.cpp: loading bar
namespace {
struct IoTimer {
    std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
    ~IoTimer() { rt_io_ns += (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0).count(); }
};
}
K32(ReadFile) {
    FILE* f = file_of(ARG(0));
    if (!f) { set_error(kErrInvalidHandle); RET(0, 5); }
    IoTimer timer;
    ++rt_io_reads;
    const size_t n = fread(rt_mem + ARG(1), 1, ARG(2), f);
    rt_io_bytes += n;
    rt_load_progress();
    if (ARG(3)) rt_wr32(ARG(3), (uint32_t)n);
    RET(1, 5);
}
K32(WriteFile) {
    FILE* f = file_of(ARG(0));
    if (!f) { set_error(kErrInvalidHandle); RET(0, 5); }
    const size_t n = fwrite(rt_mem + ARG(1), 1, ARG(2), f);
    if (ARG(3)) rt_wr32(ARG(3), (uint32_t)n);
    RET(1, 5);
}
K32(WriteConsoleA) { fwrite(rt_mem + ARG(1), 1, ARG(2), stdout); if (ARG(3)) rt_wr32(ARG(3), ARG(2)); RET(1, 5); }
K32(WriteConsoleW) {
    fputs(wide_to_utf8(ARG(1), (int)ARG(2)).c_str(), stdout);
    if (ARG(3)) rt_wr32(ARG(3), ARG(2));
    RET(1, 5);
}
K32(SetFilePointer) {
    FILE* f = file_of(ARG(0));
    if (!f) { set_error(kErrInvalidHandle); RET(0xffffffffu, 4); }
    IoTimer timer;
    ++rt_io_seeks;
    int64_t off = (int32_t)ARG(1);
    if (ARG(2)) off = (int64_t)(((uint64_t)rt_rd32(ARG(2)) << 32) | ARG(1));
    fseek(f, (long)off, ARG(3) == 0 ? SEEK_SET : ARG(3) == 1 ? SEEK_CUR : SEEK_END);
    const long pos = ftell(f);
    if (ARG(2)) rt_wr32(ARG(2), 0);
    RET((uint32_t)pos, 4);
}
K32(SetEndOfFile) { RET(1, 1); }
K32(FlushFileBuffers) { FILE* f = file_of(ARG(0)); if (f) fflush(f); RET(1, 1); }
K32(GetFileAttributesA) {
    struct stat st;
    const std::string p = rt_host_path(rt_str(ARG(0)));
    if (stat(p.c_str(), &st) != 0) { set_error(kErrFileNotFound); RET(0xffffffffu, 1); }
    RET(S_ISDIR(st.st_mode) ? 0x10 : 0x80, 1);
}
K32(GetFileAttributesW) {
    struct stat st;
    const std::string p = rt_host_path(wide_to_utf8(ARG(0)).c_str());
    if (stat(p.c_str(), &st) != 0) { set_error(kErrFileNotFound); RET(0xffffffffu, 1); }
    RET(S_ISDIR(st.st_mode) ? 0x10 : 0x80, 1);
}
K32(DeleteFileA) { RET(remove(rt_host_path(rt_str(ARG(0))).c_str()) == 0, 1); }
K32(GetCurrentDirectoryA) {
    const char* dir = "C:\\MMU";
    if (ARG(0) > strlen(dir)) strcpy((char*)rt_mem + ARG(1), dir);
    RET(strlen(dir), 2);
}
K32(SetCurrentDirectoryA) { RET(1, 1); }
K32(GetFullPathNameA) {
    std::string in = rt_str(ARG(0));
    std::string full = (in.size() > 1 && in[1] == ':') ? in : "C:\\MMU\\" + in;
    if (ARG(1) > full.size()) strcpy((char*)rt_mem + ARG(2), full.c_str());
    if (ARG(3)) rt_wr32(ARG(3), 0);
    RET(full.size(), 4);
}
K32(GetDriveTypeA) { RET(3, 1); }
K32(GetLogicalDrives) { RET(4, 0); }

// ---- console input (unused by the game)
K32(GetNumberOfConsoleInputEvents) { RET(0, 2); }
K32(PeekConsoleInputA) { RET(0, 4); }
K32(ReadConsoleInputA) { RET(0, 4); }

// ---- misc
K32(Beep) { RET(1, 2); }

// ---- version / more files / sync objects
K32(GetVersionExA) {
    // Windows 8+ report 6.2.9200 to programs without a compatibility
    // manifest (MMU.exe has none).
    const uint32_t p = ARG(0);
    const uint32_t size = rt_rd32(p);
    memset(rt_mem + p + 4, 0, size > 4 ? size - 4 : 0);
    rt_wr32(p + 4, 6);
    rt_wr32(p + 8, 2);
    rt_wr32(p + 12, 9200);
    rt_wr32(p + 16, 2); // VER_PLATFORM_WIN32_NT
    RET(1, 1);
}
#if defined(_WIN32)
K32(CreateDirectoryA) { RET(mkdir(rt_host_path(rt_str(ARG(0))).c_str()) == 0, 2); }
#else
K32(CreateDirectoryA) { RET(mkdir(rt_host_path(rt_str(ARG(0))).c_str(), 0755) == 0, 2); }
#endif
K32(RemoveDirectoryA) { RET(rmdir(rt_host_path(rt_str(ARG(0))).c_str()) == 0, 1); }
K32(MoveFileA) {
    RET(rename(rt_host_path(rt_str(ARG(0))).c_str(), rt_host_path(rt_str(ARG(1))).c_str()) == 0, 2);
}
K32(SetFileAttributesA) { RET(1, 2); }
K32(SetFileTime) { RET(1, 4); }
K32(LockFile) { RET(1, 5); }
K32(UnlockFile) { RET(1, 5); }
K32(GetFileInformationByHandle) { set_error(kErrInvalidHandle); RET(0, 2); }
K32(SystemTimeToFileTime) {
    uint16_t st[8];
    memcpy(st, rt_mem + ARG(0), 16);
    std::tm tm{};
    tm.tm_year = st[0] - 1900; tm.tm_mon = st[1] - 1; tm.tm_mday = st[3];
    tm.tm_hour = st[4]; tm.tm_min = st[5]; tm.tm_sec = st[6];
    // Days from civil (portable timegm; newlib has none).
    const int y = tm.tm_year + 1900 - (tm.tm_mon < 2), era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400), mp = (unsigned)((tm.tm_mon + 9) % 12);
    const unsigned doy = (153 * mp + 2) / 5 + (unsigned)tm.tm_mday - 1;
    const long days = (long)era * 146097 + (long)(yoe * 365 + yoe / 4 - yoe / 100 + doy) - 719468;
    const std::time_t t = (std::time_t)(days * 86400 + tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec);
    write_filetime(ARG(1), (uint64_t)t * 10000000ull + st[7] * 10000ull + 116444736000000000ull);
    RET(1, 2);
}
K32(CreateEventA) { RET(0x200, 4); }
K32(CreateMutexA) { RET(0x204, 3); }
K32(CreateSemaphoreA) { RET(0x208, 4); }
K32(ReleaseMutex) { RET(1, 1); }
K32(ReleaseSemaphore) { if (ARG(2)) rt_wr32(ARG(2), 0); RET(1, 3); }
K32(SetThreadPriority) { RET(1, 2); }
K32(FormatMessageA) { RET(0, 7); }

// FindFirstFileA/FindNextFileA over the mapped directory (WIN32_FIND_DATAA).
#include <dirent.h>
namespace {
struct FindState { DIR* dir; std::string dir_path; std::string pattern; };
std::unordered_map<uint32_t, FindState> g_finds;
bool wild_match(const char* pat, const char* s) {
    if (!*pat) return !*s;
    if (*pat == '*') return wild_match(pat + 1, s) || (*s && wild_match(pat, s + 1));
    if (*s && (*pat == '?' || tolower((unsigned char)*pat) == tolower((unsigned char)*s)))
        return wild_match(pat + 1, s + 1);
    return false;
}
bool find_next(FindState& st, uint32_t out) {
    while (dirent* e = readdir(st.dir)) {
        if (!wild_match(st.pattern.c_str(), e->d_name)) continue;
        memset(rt_mem + out, 0, 320);
        struct stat sb;
        const std::string full = st.dir_path + "/" + e->d_name;
        const bool isdir = stat(full.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
        rt_wr32(out, isdir ? 0x10 : 0x80);
        rt_wr32(out + 32, isdir ? 0 : (uint32_t)sb.st_size);
        strncpy((char*)rt_mem + out + 44, e->d_name, 259);
        return true;
    }
    return false;
}
} // namespace
K32(FindFirstFileA) {
    std::string path = rt_host_path(rt_str(ARG(0)));
    const size_t slash = path.find_last_of('/');
    FindState st{nullptr, path.substr(0, slash), path.substr(slash + 1)};
    st.dir = opendir(st.dir_path.c_str());
    if (!st.dir || !find_next(st, ARG(1))) {
        if (st.dir) closedir(st.dir);
        set_error(kErrFileNotFound);
        RET(kInvalidHandle, 2);
    }
    const uint32_t h = g_next_handle;
    g_next_handle += 4;
    g_finds[h] = st;
    RET(h, 2);
}
K32(FindNextFileA) {
    auto it = g_finds.find(ARG(0));
    if (it == g_finds.end() || !find_next(it->second, ARG(1))) { set_error(18); RET(0, 2); }
    RET(1, 2);
}
K32(FindClose) {
    auto it = g_finds.find(ARG(0));
    if (it != g_finds.end()) { closedir(it->second.dir); g_finds.erase(it); }
    RET(1, 1);
}

// ---- ADVAPI32 / WINMM (stdcall)
extern "C" void imp_ADVAPI32__RegOpenKeyExA(Cpu* c) { RET(2, 5); }
extern "C" void imp_ADVAPI32__RegQueryValueExA(Cpu* c) { RET(2, 6); }
extern "C" void imp_ADVAPI32__RegCloseKey(Cpu* c) { RET(0, 1); }
extern "C" void imp_WINMM__timeGetTime(Cpu* c) {
    using namespace std::chrono;
    RET((uint32_t)duration_cast<milliseconds>(steady_clock::now() - g_start).count(), 0);
}
extern "C" void imp_WINMM__timeBeginPeriod(Cpu* c) { RET(0, 1); }
extern "C" void imp_WINMM__timeEndPeriod(Cpu* c) { RET(0, 1); }
extern "C" void imp_WINMM__joyGetNumDevs(Cpu* c) { RET(0, 0); }
extern "C" void imp_USER32__GetKeyState(Cpu* c) { RET(0, 1); }
extern "C" void imp_USER32__GetKeyboardLayout(Cpu* c) { RET(0x04090409u, 1); }
extern "C" void imp_USER32__GetDesktopWindow(Cpu* c) { RET(0x10010, 0); }
extern "C" void imp_USER32__GetForegroundWindow(Cpu* c) { RET(0x10020, 0); }
// Native MIDI (SDL_mixer probes it): report no MIDI driver.
extern "C" void imp_WINMM__midiStreamOpen(Cpu* c) { RET(6, 6); }
extern "C" void imp_WINMM__midiStreamClose(Cpu* c) { RET(0, 1); }
extern "C" void imp_WINMM__midiOutSetVolume(Cpu* c) { RET(6, 2); }
extern "C" void imp_WINMM__mciSendCommandA(Cpu* c) { RET(6, 4); }
// GetVersion: build << 16 | minor << 8 | major (Windows 8 identity, see GetVersionExA).
K32(GetVersion) { RET((9200u << 16) | (2u << 8) | 6u, 0); }

// Lockstep snapshots: TLS slots and last error.
extern "C" void rt_k32_state(uint32_t* tls128, uint32_t* last_error) {
    memcpy(tls128, g_tls, sizeof(g_tls));
    *last_error = g_last_error;
}
