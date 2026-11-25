// Logging for diagnosing crashes and memory exhaustion, mostly on Switch 2.
//
// Switch 2 (files next to the NRO, in sdmc:/switch/mmu_recomp/):
//   log.txt   everything the runtime and the game print (stdout line-buffered,
//             stderr unbuffered on the same descriptor). Rewritten each run and
//             capped at kLogLines lines (crash reports always get through), so
//             it never grows on the player's SD card.
//   boot.txt  breadcrumbs, unbuffered with a millisecond stamp, so a hard
//             crash still shows how far the run got (kCrumbLines per run).
//   debug_log (any file of that name) turns on developer logging: no line
//             caps, the live network log (`nc <switch-ip> 28280`) and a status
//             line every 2 s. Off for players.
//   Launched with `nxlink -s mmu_recomp.nro`, stdout/stderr go to the PC's
//   terminal instead (real-time), and boot.txt is still written.
//
// Every RECOMP_STATUS seconds (Switch: 2 with debug_log, host default off) a status
// line reports frames/FPS, guest heap use and (Switch) process memory, plus
// the guest function the game was in. Crashes print the guest call ring
// (rt_dump_state) through the libnx exception handler / POSIX signals.
#include "recomp/cpu.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <initializer_list>
#include <pthread.h>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(__SWITCH__)
#include <arpa/inet.h>
#include <switch.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/iosupport.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

extern "C" uint32_t rt_heap_used(void);
extern "C" uint64_t rt_io_reads, rt_io_bytes, rt_io_seeks, rt_io_ns;
extern "C" uint32_t rt_trace_va[64], rt_trace_pos;

namespace {
FILE* g_crumbs = nullptr;
bool g_unlimited = true;        // Switch without debug_log: capped files
unsigned g_crumb_lines = 0;
constexpr unsigned kCrumbLines = 300;
auto g_t0 = std::chrono::steady_clock::now();
double g_status_every = 0.0;

double now_s() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - g_t0).count();
}

uint32_t current_guest_fn() { return rt_trace_va[(rt_trace_pos - 1u) & 63u]; }

void memory_line(char* out, size_t n) {
#if defined(__SWITCH__)
    u64 used = 0, total = 0;
    svcGetInfo(&used, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
    svcGetInfo(&total, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
    std::snprintf(out, n, "guest heap %u MiB of %u MiB, process %llu of %llu MiB", rt_heap_used() >> 20,
                  (RT_HEAP_END - RT_HEAP_BASE) >> 20, (unsigned long long)(used >> 20),
                  (unsigned long long)(total >> 20));
#else
    std::snprintf(out, n, "guest heap %u MiB of %u MiB", rt_heap_used() >> 20, (RT_HEAP_END - RT_HEAP_BASE) >> 20);
#endif
}

#if !defined(__SWITCH__)
void on_signal(int sig) {
    std::fprintf(stderr, "recomp: fatal signal %d, in guest f_%x\n", sig, current_guest_fn());
    rt_dump_state();
    std::fflush(stdout);
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}
#endif
}  // namespace

extern "C" void rt_crumb(const char* fmt, ...) {
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    if (g_crumbs && (g_unlimited || g_crumb_lines < kCrumbLines)) {
        ++g_crumb_lines;
        std::fprintf(g_crumbs, "%8.3f s %s\n", now_s(), line);
        if (!g_unlimited && g_crumb_lines == kCrumbLines) std::fprintf(g_crumbs, "[boot.txt limit reached]\n");
        std::fflush(g_crumbs);
    }
    std::fprintf(stderr, "[%.3f] %s\n", now_s(), line);
}

extern "C" void rt_log_memory(const char* why) {
    char m[160];
    memory_line(m, sizeof m);
    rt_crumb("%s: %s, in guest f_%x", why, m, current_guest_fn());
}

extern "C" void rt_log_status(unsigned frames) {
    if (g_status_every <= 0.0) return;
    static double next = 0.0;
    static unsigned last_frames = 0;
    static double last_t = 0.0;
    const double t = now_s();
    if (t < next) return;
    next = t + g_status_every;
    char m[160];
    memory_line(m, sizeof m);
    const double fps = t > last_t ? (frames - last_frames) / (t - last_t) : 0.0;
    std::printf("[%.1f] status: frame %u (%.1f fps), %s, io %llu reads %llu MiB %llu seeks %.2fs, guest f_%x\n", t,
                frames, fps, m, (unsigned long long)rt_io_reads, (unsigned long long)(rt_io_bytes >> 20),
                (unsigned long long)rt_io_seeks, rt_io_ns / 1e9, current_guest_fn());
    std::fflush(stdout);
    last_frames = frames;
    last_t = t;
}

#if defined(__SWITCH__)
// ---- Network log (the console cannot run an FTP server next to the game).
// stdout/stderr go through the "mmulog:" device, which appends to log.txt and
// sends to every client connected to TCP port RT_LOG_PORT
// (`nc <switch-ip> 28280`, tools/recomp/watch_switch_log.sh). A client gets
// the last 64 KiB of output first, so connecting late still shows the start.
#define RT_LOG_PORT 28280
extern "C" long gethostid(void);
namespace {
int g_log_fd = -1;
int g_clients[4] = {-1, -1, -1, -1};
pthread_mutex_t g_net_lock = PTHREAD_MUTEX_INITIALIZER;
bool g_crashing = false;
char g_ring[64 * 1024];
size_t g_ring_pos = 0, g_ring_len = 0;
constexpr unsigned kLogLines = 2000;  // per run, without debug_log
unsigned g_log_lines = 0;

void file_write(const char* ptr, size_t len) {
    if (g_log_fd < 0) return;
    if (g_unlimited || g_crashing) { write(g_log_fd, ptr, len); return; }
    if (g_log_lines >= kLogLines) return;
    size_t n = 0;
    while (n < len && g_log_lines < kLogLines)
        if (ptr[n++] == '\n') ++g_log_lines;
    write(g_log_fd, ptr, n);
    if (g_log_lines >= kLogLines) {
        static const char kMsg[] = "[log.txt limit reached; crash reports are still written]\n";
        write(g_log_fd, kMsg, sizeof kMsg - 1);
    }
}

void send_all(int fd, const char* p, size_t n) {
    while (n) {
        const ssize_t k = send(fd, p, n, 0);
        if (k <= 0) return;
        p += k;
        n -= (size_t)k;
    }
}

ssize_t tee_write(struct _reent*, void*, const char* ptr, size_t len) {
    file_write(ptr, len);
    if (!g_unlimited) return (ssize_t)len;  // no network log for players
    const bool locked = !g_crashing && pthread_mutex_lock(&g_net_lock) == 0;
    for (size_t i = 0; i < len; ++i) {
        g_ring[g_ring_pos] = ptr[i];
        g_ring_pos = (g_ring_pos + 1) % sizeof g_ring;
    }
    g_ring_len = g_ring_len + len > sizeof g_ring ? sizeof g_ring : g_ring_len + len;
    for (int& c : g_clients) {
        if (c < 0) continue;
        if (send(c, ptr, len, 0) < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            close(c);
            c = -1;
        }
    }
    if (locked) pthread_mutex_unlock(&g_net_lock);
    return (ssize_t)len;
}
int tee_open(struct _reent*, void*, const char*, int, int) { return 0; }
int tee_close(struct _reent*, void*) { return 0; }
devoptab_t g_tee_dev;

void* log_server(void*) {
    const int ls = socket(AF_INET, SOCK_STREAM, 0);
    if (ls < 0) return nullptr;
    int one = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(RT_LOG_PORT);
    a.sin_addr.s_addr = INADDR_ANY;
    if (bind(ls, (sockaddr*)&a, sizeof a) < 0 || listen(ls, 2) < 0) {
        std::printf("log server: cannot listen on port %d\n", RT_LOG_PORT);
        close(ls);
        return nullptr;
    }
    for (;;) {
        const int c = accept(ls, nullptr, nullptr);
        if (c < 0) { svcSleepThread(100000000ull); continue; }
        pthread_mutex_lock(&g_net_lock);
        // History first (oldest byte at g_ring_pos once the ring is full).
        const size_t start = (g_ring_pos + sizeof g_ring - g_ring_len) % sizeof g_ring;
        const size_t first = g_ring_len < sizeof g_ring - start ? g_ring_len : sizeof g_ring - start;
        send_all(c, g_ring + start, first);
        send_all(c, g_ring, g_ring_len - first);
        fcntl(c, F_SETFL, fcntl(c, F_GETFL, 0) | O_NONBLOCK);  // a slow client never stalls the game
        bool placed = false;
        for (int& slot : g_clients)
            if (slot < 0) { slot = c; placed = true; break; }
        if (!placed) close(c);
        pthread_mutex_unlock(&g_net_lock);
    }
}

// stdout/stderr -> log.txt (+ network). Falls back to a plain file.
void open_log(const char* path, bool network) {
    g_log_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    g_tee_dev.name = "mmulog";
    g_tee_dev.structSize = sizeof(int);
    g_tee_dev.open_r = tee_open;
    g_tee_dev.close_r = tee_close;
    g_tee_dev.write_r = tee_write;
    FILE* f = AddDevice(&g_tee_dev) >= 0 ? std::freopen("mmulog:/", "w", stdout) : nullptr;
    if (!f) f = std::freopen(path, "w", stdout);
    if (!f) return;
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    // dup2 does not redirect on Horizon: point stderr's FILE at the same descriptor.
    std::fflush(stderr);
    stderr->_file = stdout->_file;
    std::setvbuf(stderr, nullptr, _IONBF, 0);
    if (network) {
        pthread_t t;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 64 * 1024);
        if (pthread_create(&t, &attr, log_server, nullptr) == 0) pthread_detach(t);
    }
}
}  // namespace

// Called by libnx/libounce on a CPU exception (if the kernel delivers it).
extern "C" {
alignas(16) u8 __nx_exception_stack[0x4000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
extern char __start__[];
}

extern "C" void __libnx_exception_handler(ThreadExceptionDump* ctx) {
    const uintptr_t base = (uintptr_t)__start__;
    g_crashing = true;  // the faulting thread may hold the network lock
    std::fprintf(stderr, "recomp: CPU exception 0x%x (pc %#llx = nro+%#llx, lr nro+%#llx, far %#llx)\n",
                 ctx->error_desc, (unsigned long long)ctx->pc.x, (unsigned long long)(ctx->pc.x - base),
                 (unsigned long long)(ctx->lr.x - base), (unsigned long long)ctx->far.x);
    const uintptr_t far = ctx->far.x, mem = (uintptr_t)rt_mem;
    if (far - mem < (uintptr_t)RT_GUEST_SIZE + 0x100000000ull)
        std::fprintf(stderr, "  fault address is guest 0x%llx\n", (unsigned long long)(far - mem));
    for (int i = 0; i < 29; i += 4)
        std::fprintf(stderr, "  x%-2d %016llx %016llx %016llx %016llx\n", i, (unsigned long long)ctx->cpu_gprs[i].x,
                     (unsigned long long)ctx->cpu_gprs[i + 1 < 29 ? i + 1 : i].x,
                     (unsigned long long)ctx->cpu_gprs[i + 2 < 29 ? i + 2 : i].x,
                     (unsigned long long)ctx->cpu_gprs[i + 3 < 29 ? i + 3 : i].x);
    rt_dump_state();
    rt_log_memory("crash");
    std::fflush(stdout);
    if (g_crumbs) std::fclose(g_crumbs);
    svcExitProcess();
}
#endif

// ---- Sampling profiler (debug_log on Switch, RECOMP_PROFILE=1 on desktop):
// every millisecond, note the guest function most recently entered; every
// 10 s print the top ones. Finds what a slow scene spends its time on.
namespace {
void* profiler_thread(void*) {
    std::unordered_map<uint32_t, unsigned> hits;
    unsigned total = 0;
    auto last = std::chrono::steady_clock::now();
    for (;;) {
#if defined(__SWITCH__)
        svcSleepThread(1000000ull);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
        ++hits[rt_trace_va[(rt_trace_pos - 1u) & 63u]];
        ++total;
        if (std::chrono::steady_clock::now() - last < std::chrono::seconds(10)) continue;
        last = std::chrono::steady_clock::now();
        std::vector<std::pair<unsigned, uint32_t>> top;
        for (const auto& h : hits) top.push_back({h.second, h.first});
        std::sort(top.rbegin(), top.rend());
        std::printf("[%.1f] profile (%u samples):", now_s(), total);
        for (size_t i = 0; i < top.size() && i < 10; ++i)
            std::printf(" f_%x %.1f%%", top[i].second, 100.0 * top[i].first / total);
        std::printf("\n");
        hits.clear();
        total = 0;
    }
    return nullptr;
}
void start_profiler() {
    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 256 * 1024);
    if (pthread_create(&t, &attr, profiler_thread, nullptr) == 0) pthread_detach(t);
}
}  // namespace

extern "C" void rt_log_init(const char* log_dir) {
    if (const char* s = std::getenv("RECOMP_STATUS")) g_status_every = std::atof(s);
#if defined(__SWITCH__)
    mkdir(log_dir, 0777);
    char path[512];
    struct stat st;
    std::snprintf(path, sizeof path, "%s/debug_log", log_dir);
    const bool debug = stat(path, &st) == 0;
    g_unlimited = debug;
    if (debug && !std::getenv("RECOMP_STATUS")) g_status_every = 2.0;
    std::snprintf(path, sizeof path, "%s/boot.txt", log_dir);
    g_crumbs = std::fopen(path, "w");
    if (g_crumbs) {
        std::setvbuf(g_crumbs, nullptr, _IONBF, 0);
        std::fprintf(g_crumbs, "=== run (tick %llu)\n", (unsigned long long)armGetSystemTick());
    }
    // Sockets only for developers: debug_log, or launched by nxlink.
    const bool net = (debug || __nxlink_host.s_addr) && R_SUCCEEDED(socketInitializeDefault());
    bool nxlink = false;
    if (net && __nxlink_host.s_addr) nxlink = nxlinkConnectToHost(true, true) >= 0;
    if (!nxlink) {
        std::snprintf(path, sizeof path, "%s/log.txt", log_dir);
        open_log(path, net && debug);
    }
    std::printf("\n=== mmu_recomp %s %s (%s)\n", __DATE__, __TIME__, nxlink ? "nxlink" : "log.txt");
    if (net && debug && !nxlink) {
        const uint32_t ip = (uint32_t)gethostid();  // network byte order
        rt_crumb("live log: nc %u.%u.%u.%u %d", ip & 255, (ip >> 8) & 255, (ip >> 16) & 255, ip >> 24, RT_LOG_PORT);
    }
    if (debug) start_profiler();
    std::printf("applet type %d (0 = application: full memory; 2/3 = applet mode: little memory)\n",
                (int)appletGetAppletType());
    rt_log_memory("start");
#else
    (void)log_dir;
    for (int s : {SIGSEGV, SIGILL, SIGFPE, SIGABRT}) std::signal(s, on_signal);
#if defined(SIGBUS)
    std::signal(SIGBUS, on_signal);
#endif
    if (std::getenv("RECOMP_PROFILE")) start_profiler();
#endif
}
