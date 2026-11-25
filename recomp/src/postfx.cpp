// Screen shaders and colour filters for the final present: a port of
// OpenHCL-Switch2's postfx.cpp (same modes, filters and GL path) on the 256x224
// game target. Shaders: perfect-retroshaders (MIT, recomp/shaders, embedded by
// CMake). GL drawing needs SDL's opengles2 renderer (Switch 2); elsewhere the
// settings work but rt_postfx_draw returns false (plain SDL presentation).
// Port additions: per-shader parameter editing (the port menu) on top of each
// mode's overrides, saved in mmu_port.ini.
#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "postfx_shaders.h"  // kShaderPixel / kShaderCrt / kShaderLcd / kShaderDmg

namespace {

enum Correction { CORRECT_NONE = 0, CORRECT_GBC = 1 };

struct ShaderMode {
	const char* label;
	int program;                  // index into kProgramFiles
	const char* overrides;        // "name=value;..." on top of the file's defaults
	int correction;               // screen colour response, after the filter
};

enum { PROG_PIXEL = 0, PROG_CRT, PROG_LCD, PROG_DMG, PROG_COUNT };
const char* kProgramFiles[PROG_COUNT] = {
	"shaders/pixel-perfect.glsl", "shaders/crt-perfect.glsl",
	"shaders/lcd-perfect.glsl", "shaders/dmg-perfect.glsl",
};

const ShaderMode kShaders[] = {
	{"NONE",       PROG_PIXEL, "", CORRECT_NONE},
	{"SCANLINES",  PROG_CRT,   "", CORRECT_NONE},
	// A small tube TV: curved glass, a slightly stronger mask.
	{"CRT TUBE",   PROG_CRT,   "cp_curvature=0.10;cp_rgb_mask=0.30;cp_scanlines=0.65", CORRECT_NONE},
	// Every source pixel outlined by a thin dark grid, no subpixel stripes.
	{"PIXEL GRID", PROG_LCD,   "lp_grid=0.55;lp_balance=0.50;lp_subpixels=0.0;lp_brightness=1.15", CORRECT_NONE},
	// Original Game Boy: dot matrix with light gaps and the dots' cast shadow.
	{"DMG LCD",    PROG_DMG,   "dp_grid=0.35;dp_shadow=0.35", CORRECT_NONE},
	// Game Boy Color: a fine LCD grid plus the GBC screen's colour response.
	{"GBC LCD",    PROG_LCD,   "lp_grid=0.30;lp_subpixels=0.15", CORRECT_GBC},
	// Master System on a TV: slot-mask CRT, softer scanlines.
	{"SMS TV",     PROG_CRT,   "cp_mask_type=2;cp_rgb_mask=0.35;cp_scanlines=0.45;cp_brightness=1.30", CORRECT_NONE},
};
const int kShaderCount = sizeof(kShaders) / sizeof(kShaders[0]);

enum FilterKind { FILTER_NONE = 0, FILTER_PALETTE = 1, FILTER_12BIT = 2 };

struct ColorFilter {
	const char* label;
	int kind;
	unsigned pal[4];  // lightest -> darkest, 0xRRGGBB
};

// Super Game Boy built-in palettes 1-A..4-H (background colours, from the
// Analogue Pocket .pal set in davewongillies/openfpga-palettes).
const ColorFilter kFilters[] = {
	{"NONE",        FILTER_NONE,    {0, 0, 0, 0}},
	{"GB SEPIA",    FILTER_PALETTE, {0xE8D8B0, 0xB89868, 0x786040, 0x382818}},
	{"GB MONO",     FILTER_PALETTE, {0x9BBC0F, 0x8BAC0F, 0x306230, 0x0F380F}},
	{"GAME GEAR",   FILTER_12BIT,   {0, 0, 0, 0}},
	{"VIRTUAL BOY", FILTER_PALETTE, {0xFF0000, 0xAA0000, 0x550000, 0x000000}},
	{"SGB 1-A", FILTER_PALETTE, {0xffefce, 0xde944a, 0xad2921, 0x311852}},
	{"SGB 1-B", FILTER_PALETTE, {0xdedec6, 0xceb573, 0xb55210, 0x000000}},
	{"SGB 1-C", FILTER_PALETTE, {0xffc6ff, 0xef9c52, 0x9c3963, 0x39399c}},
	{"SGB 1-D", FILTER_PALETTE, {0xffffad, 0xc6844a, 0xff0000, 0x521800}},
	{"SGB 1-E", FILTER_PALETTE, {0xffdeb5, 0x7bc67b, 0x6b8c42, 0x5a3921}},
	{"SGB 1-F", FILTER_PALETTE, {0xdeefff, 0xe78c52, 0xad0000, 0x004210}},
	{"SGB 1-G", FILTER_PALETTE, {0x000052, 0x00a5ef, 0x7b7b00, 0xffff5a}},
	{"SGB 1-H", FILTER_PALETTE, {0xffefe7, 0xffbd8c, 0x844200, 0x311800}},
	{"SGB 2-A", FILTER_PALETTE, {0xf7cea5, 0xc68c4a, 0x297b00, 0x000000}},
	{"SGB 2-B", FILTER_PALETTE, {0xffffff, 0xffef52, 0xff3100, 0x52005a}},
	{"SGB 2-C", FILTER_PALETTE, {0xffc6ff, 0xef8c8c, 0x7b31ef, 0x29299c}},
	{"SGB 2-D", FILTER_PALETTE, {0xffffa5, 0x00ff00, 0xff3100, 0x000052}},
	{"SGB 2-E", FILTER_PALETTE, {0xffce84, 0x94b5e7, 0x291063, 0x100810}},
	{"SGB 2-F", FILTER_PALETTE, {0xd6ffff, 0xff9452, 0xa50000, 0x180000}},
	{"SGB 2-G", FILTER_PALETTE, {0x6bbd39, 0xe75242, 0xe7bd84, 0x001800}},
	{"SGB 2-H", FILTER_PALETTE, {0xffffff, 0xbdbdbd, 0x737373, 0x000000}},
	{"SGB 3-A", FILTER_PALETTE, {0xffd69c, 0x73c6c6, 0xff6329, 0x314a63}},
	{"SGB 3-B", FILTER_PALETTE, {0xdedec6, 0xe78421, 0x005200, 0x001010}},
	{"SGB 3-C", FILTER_PALETTE, {0xe7adce, 0xffff7b, 0x00bdff, 0x21215a}},
	{"SGB 3-D", FILTER_PALETTE, {0xf7ffbd, 0xe7ad7b, 0x08ce00, 0x000000}},
	{"SGB 3-E", FILTER_PALETTE, {0xffffc6, 0xe7b56b, 0xb57b21, 0x524a73}},
	{"SGB 3-F", FILTER_PALETTE, {0x7b7bce, 0xff6bff, 0xffd600, 0x424242}},
	{"SGB 3-G", FILTER_PALETTE, {0x63de52, 0xffffff, 0xce3139, 0x390000}},
	{"SGB 3-H", FILTER_PALETTE, {0xe7ffa5, 0x7bce39, 0x4a8c18, 0x081800}},
	{"SGB 4-A", FILTER_PALETTE, {0xf7ad6b, 0x7badff, 0xd600d6, 0x00007b}},
	{"SGB 4-B", FILTER_PALETTE, {0xf7eff7, 0xefa563, 0x427b39, 0x180808}},
	{"SGB 4-C", FILTER_PALETTE, {0xffe7e7, 0xdea5d6, 0x9ca5e7, 0x080000}},
	{"SGB 4-D", FILTER_PALETTE, {0xffffbd, 0x94cece, 0x4a6b7b, 0x08214a}},
	{"SGB 4-E", FILTER_PALETTE, {0xffdead, 0xe7ad7b, 0x7b5a8c, 0x002131}},
	{"SGB 4-F", FILTER_PALETTE, {0xbdd6d6, 0xde84de, 0x8400a5, 0x390000}},
	{"SGB 4-G", FILTER_PALETTE, {0xb5e718, 0xbd215a, 0x291000, 0x008463}},
	{"SGB 4-H", FILTER_PALETTE, {0xffffce, 0xbdc65a, 0x848c42, 0x425229}},
};
const int kFilterCount = sizeof(kFilters) / sizeof(kFilters[0]);

int g_shader = 0;
int g_filter = 0;

const char* embedded_shader(const char* file) {
	if (std::strstr(file, "pixel-perfect")) return kShaderPixel;
	if (std::strstr(file, "crt-perfect")) return kShaderCrt;
	if (std::strstr(file, "lcd-perfect")) return kShaderLcd;
	if (std::strstr(file, "dmg-perfect")) return kShaderDmg;
	return nullptr;
}

// ---- parameters ("#pragma parameter name "desc" default min max step")
struct ParamInfo { std::string name, desc; float def, lo, hi, step; };
std::vector<ParamInfo> g_param_cache[PROG_COUNT];
bool g_param_parsed[PROG_COUNT] = {};
const std::vector<ParamInfo>& program_params(int prog) {
	if (!g_param_parsed[prog]) {
		g_param_parsed[prog] = true;
		const std::string src = embedded_shader(kProgramFiles[prog]);
		for (size_t at = 0; (at = src.find("#pragma parameter", at)) != std::string::npos;) {
			const size_t end = src.find('\n', at);
			const std::string line = src.substr(at + 17, end - at - 17);
			at = end;
			char name[64];
			if (std::sscanf(line.c_str(), " %63s", name) != 1) continue;
			const size_t q1 = line.find('"');
			const size_t q2 = q1 == std::string::npos ? q1 : line.find('"', q1 + 1);
			if (q2 == std::string::npos) continue;
			ParamInfo p{name, line.substr(q1 + 1, q2 - q1 - 1), 0, 0, 1, 0.01f};
			std::sscanf(line.c_str() + q2 + 1, "%f %f %f %f", &p.def, &p.lo, &p.hi, &p.step);
			g_param_cache[prog].push_back(p);
		}
	}
	return g_param_cache[prog];
}
// User values per shader mode (name -> value), on top of the mode's overrides.
std::map<std::string, float> g_user[sizeof(kShaders) / sizeof(kShaders[0])];
float overrideValue(const char* overrides, const std::string& name, float def);
float param_value(int mode, const std::string& name, float fallback) {
	auto it = g_user[mode].find(name);
	return it != g_user[mode].end() ? it->second : fallback;
}

}  // namespace

extern "C" int rt_postfx_mode(void) { return g_shader; }
extern "C" int rt_postfx_count(void) { return kShaderCount; }
extern "C" const char* rt_postfx_mode_label(int n) { return kShaders[n].label; }
extern "C" void rt_postfx_set(int n) { g_shader = ((n % kShaderCount) + kShaderCount) % kShaderCount; }
extern "C" int rt_postfx_filter(void) { return g_filter; }
extern "C" int rt_postfx_filter_count(void) { return kFilterCount; }
extern "C" const char* rt_postfx_filter_label(int n) { return kFilters[n].label; }
extern "C" void rt_postfx_set_filter(int n) { g_filter = ((n % kFilterCount) + kFilterCount) % kFilterCount; }
// Parameters of the current shader mode (none for NONE).
extern "C" int rt_postfx_param_count(void) {
	return g_shader == 0 ? 0 : (int)program_params(kShaders[g_shader].program).size();
}
extern "C" void rt_postfx_param_info(int i, const char** desc, float* lo, float* hi, float* step) {
	const ParamInfo& p = program_params(kShaders[g_shader].program)[(size_t)i];
	*desc = p.desc.c_str(); *lo = p.lo; *hi = p.hi; *step = p.step;
}
extern "C" float rt_postfx_param_get(int i) {
	const ParamInfo& p = program_params(kShaders[g_shader].program)[(size_t)i];
	return param_value(g_shader, p.name, overrideValue(kShaders[g_shader].overrides, p.name, p.def));
}
extern "C" void rt_postfx_param_set(int i, float v) {
	g_user[g_shader][program_params(kShaders[g_shader].program)[(size_t)i].name] = v;
}
extern "C" void rt_postfx_param_reset(void) { g_user[g_shader].clear(); }
// mmu_port.ini lines: "shader_param=<mode> <name> <value>".
extern "C" void rt_postfx_save(FILE* f) {
	for (int m = 0; m < kShaderCount; ++m)
		for (const auto& kv : g_user[m]) std::fprintf(f, "shader_param=%d %s %g\n", m, kv.first.c_str(), kv.second);
}
extern "C" void rt_postfx_load_line(const char* line) {
	int m;
	char name[64];
	float v;
	if (std::sscanf(line, "shader_param=%d %63s %f", &m, name, &v) == 3 && m >= 0 && m < kShaderCount) g_user[m][name] = v;
}

#if !defined(__SWITCH__)
namespace {
float overrideValue(const char* overrides, const std::string& name, float def) {
	for (const char* s = overrides; s && *s;) {
		const char* eq = std::strchr(s, '=');
		if (!eq) break;
		const char* end = std::strchr(eq, ';');
		if (std::string(s, (size_t)(eq - s)) == name) return (float)std::atof(eq + 1);
		s = end ? end + 1 : nullptr;
	}
	return def;
}
}  // namespace
extern "C" void rt_postfx_finish(SDL_Renderer*) {}
extern "C" bool rt_postfx_draw(SDL_Renderer*, SDL_Texture*, const SDL_Rect*) { return false; }
#else
#define GL_GLES_PROTOTYPES 0
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// ---------------------------------------------------------------------------
// GL

namespace {

#define GLFN(name, type) type name = nullptr;
#define GL_FUNCS(X) \
	X(glActiveTexture, PFNGLACTIVETEXTUREPROC) \
	X(glAttachShader, PFNGLATTACHSHADERPROC) \
	X(glBindTexture, PFNGLBINDTEXTUREPROC) \
	X(glBindAttribLocation, PFNGLBINDATTRIBLOCATIONPROC) \
	X(glBindBuffer, PFNGLBINDBUFFERPROC) \
	X(glBufferData, PFNGLBUFFERDATAPROC) \
	X(glCompileShader, PFNGLCOMPILESHADERPROC) \
	X(glCreateProgram, PFNGLCREATEPROGRAMPROC) \
	X(glCreateShader, PFNGLCREATESHADERPROC) \
	X(glDeleteShader, PFNGLDELETESHADERPROC) \
	X(glDisable, PFNGLDISABLEPROC) \
	X(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC) \
	X(glDrawArrays, PFNGLDRAWARRAYSPROC) \
	X(glEnable, PFNGLENABLEPROC) \
	X(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC) \
	X(glGenBuffers, PFNGLGENBUFFERSPROC) \
	X(glGetIntegerv, PFNGLGETINTEGERVPROC) \
	X(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC) \
	X(glGetProgramiv, PFNGLGETPROGRAMIVPROC) \
	X(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC) \
	X(glGetShaderiv, PFNGLGETSHADERIVPROC) \
	X(glGetTexParameteriv, PFNGLGETTEXPARAMETERIVPROC) \
	X(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC) \
	X(glGetVertexAttribPointerv, PFNGLGETVERTEXATTRIBPOINTERVPROC) \
	X(glGetVertexAttribiv, PFNGLGETVERTEXATTRIBIVPROC) \
	X(glIsEnabled, PFNGLISENABLEDPROC) \
	X(glLinkProgram, PFNGLLINKPROGRAMPROC) \
	X(glShaderSource, PFNGLSHADERSOURCEPROC) \
	X(glTexParameteri, PFNGLTEXPARAMETERIPROC) \
	X(glUniform1f, PFNGLUNIFORM1FPROC) \
	X(glUniform1i, PFNGLUNIFORM1IPROC) \
	X(glUniform2f, PFNGLUNIFORM2FPROC) \
	X(glUniform3fv, PFNGLUNIFORM3FVPROC) \
	X(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC) \
	X(glUseProgram, PFNGLUSEPROGRAMPROC) \
	X(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC) \
	X(glViewport, PFNGLVIEWPORTPROC)
GL_FUNCS(GLFN)

bool g_glLoaded = false;
bool g_glFailed = false;

struct Program {
	bool tried = false;
	GLuint id = 0;
	GLint uMVP = -1, uOutputSize = -1, uTextureSize = -1, uInputSize = -1;
	GLint uFrameCount = -1, uFrameDirection = -1, uTexture = -1;
	GLint uFilterMode = -1, uFilterPal = -1, uSwapRB = -1, uCorrect = -1;
	bool filterHooked = false;
	struct Param { std::string name; float def; GLint loc; };
	std::vector<Param> params;
};
Program g_programs[PROG_COUNT];
GLuint g_vbo = 0;
int g_frameCount = 0;

// Attribute locations bound before linking. SDL sets its own attribute
// pointers on every draw, but the enables/pointers are restored anyway.
enum { ATTR_VERTEX = 0, ATTR_TEXCOORD = 1, ATTR_COLOR = 2, ATTR_SAVED = 3 };

bool loadGL() {
	if (g_glLoaded)
		return true;
	if (g_glFailed)
		return false;
#define GLLOAD(name, type) \
	name = (type)SDL_GL_GetProcAddress(#name); \
	if (!name) { std::fprintf(stderr, "postfx: missing %s\n", #name); g_glFailed = true; return false; }
	GL_FUNCS(GLLOAD)
#undef GLLOAD
	g_glLoaded = true;
	return true;
}

bool readFile(const char* rel, std::string& out) {
	// Embedded at build time (recomp/shaders -> postfx_shaders.h).
	const char* text = embedded_shader(rel);
	if (!text)
		return false;
	out = text;
	return true;
}

// Inserted ahead of the fragment shader's main(). Every texture read goes
// through hcl_tex, which swaps red/blue back when SDL stored the texture in
// a swapped channel order (see postfx_Draw). hcl_filter applies the
// colour filter and then the screen's colour response.
const char* kFilterGlsl =
	"uniform int hcl_swap_rb;\n"
	"uniform int hcl_filter_mode;\n"
	"uniform int hcl_correct;\n"
	"uniform vec3 hcl_filter_pal[4];\n"
	"vec4 hcl_tex(sampler2D s, vec2 uv) {\n"
	"    vec4 c = texture2D(s, uv);\n"
	"    return hcl_swap_rb == 1 ? c.bgra : c;\n"
	"}\n"
	"#undef COMPAT_TEXTURE\n"
	"#define COMPAT_TEXTURE hcl_tex\n"
	"vec3 hcl_correct_color(vec3 c) {\n"
	"    if (hcl_correct == 1)\n"
	"        return vec3(dot(c, vec3(13.0, 2.0, 1.0)) / 16.0,\n"
	"                    dot(c, vec3(0.0, 3.0, 1.0)) / 4.0,\n"
	"                    dot(c, vec3(3.0, 2.0, 11.0)) / 16.0);\n"
	"    return c;\n"
	"}\n"
	"vec3 hcl_palette(vec3 c) {\n"
	"    if (hcl_filter_mode == 1) {\n"
	"        float l = dot(c, vec3(0.299, 0.587, 0.114));\n"
	"        if (l >= 0.75) return hcl_filter_pal[0];\n"
	"        if (l >= 0.50) return hcl_filter_pal[1];\n"
	"        if (l >= 0.25) return hcl_filter_pal[2];\n"
	"        return hcl_filter_pal[3];\n"
	"    }\n"
	"    if (hcl_filter_mode == 2)\n"
	"        return floor(c * 15.0 + 0.5) / 15.0;\n"
	"    return c;\n"
	"}\n"
	"vec3 hcl_filter(vec3 c) {\n"
	"    return hcl_correct_color(hcl_palette(c));\n"
	"}\n";

// Patches a RetroArch-style shader: kFilterGlsl before the fragment main()
// (always, for the channel order), and `x = hcl_filter(x);` right after that
// main's first texture read (`vec3 x = COMPAT_TEXTURE(...)...;`). Returns
// false if there is no main(); *filterHooked says whether filters apply.
bool patchShader(std::string& src, bool* filterHooked) {
	*filterHooked = false;
	size_t mainPos = src.rfind("void main()");
	if (mainPos == std::string::npos)
		return false;
	size_t tap = src.find("= COMPAT_TEXTURE(", mainPos);
	if (tap == std::string::npos) {
		src.insert(mainPos, kFilterGlsl);
		return true;
	}
	size_t lineStart = src.rfind('\n', tap);
	lineStart = lineStart == std::string::npos ? 0 : lineStart + 1;
	size_t lineEnd = src.find('\n', tap);
	if (lineEnd == std::string::npos) {
		src.insert(mainPos, kFilterGlsl);
		return true;
	}
	// "    vec3 color " -> "color"
	std::string decl = src.substr(lineStart, tap - lineStart);
	size_t e = decl.find_last_not_of(" \t");
	if (e == std::string::npos) {
		src.insert(mainPos, kFilterGlsl);
		return true;
	}
	size_t b = decl.find_last_of(" \t", e);
	std::string var = decl.substr(b == std::string::npos ? 0 : b + 1, e - (b == std::string::npos ? 0 : b + 1) + 1);
	if (!var.empty() && decl.find("vec3") != std::string::npos) {
		src.insert(lineEnd + 1, "    " + var + " = hcl_filter(" + var + ");\n");
		*filterHooked = true;
	}
	src.insert(mainPos, kFilterGlsl);
	return true;
}

GLuint compile(GLenum type, const std::string& src, const char* name) {
	GLuint s = glCreateShader(type);
	const char* text = src.c_str();
	glShaderSource(s, 1, &text, nullptr);
	glCompileShader(s);
	GLint ok = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[1024] = {0};
		glGetShaderInfoLog(s, sizeof(log) - 1, nullptr, log);
		std::fprintf(stderr, "postfx: %s %s shader failed: %s\n", name, type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
		glDeleteShader(s);
		return 0;
	}
	return s;
}

bool build(int index) {
	Program& p = g_programs[index];
	if (p.tried)
		return p.id != 0;
	p.tried = true;
	const char* file = kProgramFiles[index];
	std::string src;
	if (!readFile(file, src)) {
		std::fprintf(stderr, "postfx: no embedded %s\n", file);
		return false;
	}

	// "#pragma parameter name "desc" default min max step" -> uniform defaults.
	size_t at = 0;
	while ((at = src.find("#pragma parameter", at)) != std::string::npos) {
		size_t end = src.find('\n', at);
		std::string line = src.substr(at + 17, end - at - 17);
		at = end;
		char name[64];
		if (sscanf(line.c_str(), " %63s", name) != 1)
			continue;
		size_t q = line.find('"');
		q = q == std::string::npos ? std::string::npos : line.find('"', q + 1);
		if (q == std::string::npos)
			continue;
		Program::Param param = {name, static_cast<float>(atof(line.c_str() + q + 1)), -1};
		p.params.push_back(param);
	}

	std::string frag = src;
	if (!patchShader(frag, &p.filterHooked)) {
		std::fprintf(stderr, "postfx: %s: no fragment main(); not usable\n", file);
		return false;
	}
	if (!p.filterHooked)
		std::fprintf(stderr, "postfx: %s: no texture read to hook; colour filters disabled for it\n", file);

	const char* header = "#version 100\n#define PARAMETER_UNIFORM\n";
	GLuint vs = compile(GL_VERTEX_SHADER, std::string(header) + "#define VERTEX\n" + src, file);
	GLuint fs = compile(GL_FRAGMENT_SHADER, std::string(header) + "#define FRAGMENT\n" + frag, file);
	if (!vs || !fs) {
		if (vs) glDeleteShader(vs);
		if (fs) glDeleteShader(fs);
		return false;
	}
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, ATTR_VERTEX, "VertexCoord");
	glBindAttribLocation(prog, ATTR_TEXCOORD, "TexCoord");
	glBindAttribLocation(prog, ATTR_COLOR, "COLOR");
	glLinkProgram(prog);
	glDeleteShader(vs);
	glDeleteShader(fs);
	GLint ok = 0;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[1024] = {0};
		glGetProgramInfoLog(prog, sizeof(log) - 1, nullptr, log);
		std::fprintf(stderr, "postfx: %s link failed: %s\n", file, log);
		return false;
	}
	p.id = prog;
	p.uMVP = glGetUniformLocation(prog, "MVPMatrix");
	p.uOutputSize = glGetUniformLocation(prog, "OutputSize");
	p.uTextureSize = glGetUniformLocation(prog, "TextureSize");
	p.uInputSize = glGetUniformLocation(prog, "InputSize");
	p.uFrameCount = glGetUniformLocation(prog, "FrameCount");
	p.uFrameDirection = glGetUniformLocation(prog, "FrameDirection");
	p.uTexture = glGetUniformLocation(prog, "Texture");
	p.uFilterMode = glGetUniformLocation(prog, "hcl_filter_mode");
	p.uFilterPal = glGetUniformLocation(prog, "hcl_filter_pal");
	p.uSwapRB = glGetUniformLocation(prog, "hcl_swap_rb");
	p.uCorrect = glGetUniformLocation(prog, "hcl_correct");
	for (size_t i = 0; i < p.params.size(); i++)
		p.params[i].loc = glGetUniformLocation(prog, p.params[i].name.c_str());
	std::fprintf(stderr, "postfx: built %s (%d parameters, filter %s)\n", file,
	             static_cast<int>(p.params.size()), p.filterHooked ? "hooked" : "off");
	return true;
}

float overrideValue(const char* overrides, const std::string& name, float def) {
	const char* s = overrides;
	while (s && *s) {
		const char* eq = strchr(s, '=');
		if (!eq)
			break;
		const char* end = strchr(eq, ';');
		if (std::string(s, eq - s) == name)
			return static_cast<float>(atof(eq + 1));
		s = end ? end + 1 : nullptr;
	}
	return def;
}

struct AttribState {
	GLint enabled, size, type, normalized, stride, buffer;
	void* pointer;
};

}  // namespace

static void postfx_Reset() {
	for (int i = 0; i < PROG_COUNT; i++)
		g_programs[i] = Program();
	g_vbo = 0;
}

static bool postfx_Draw(SDL_Renderer* renderer, SDL_Texture* tex, const SDL_Rect& viewport) {
	const ShaderMode& mode = kShaders[g_shader];
	const ColorFilter& filter = kFilters[g_filter];
	// Both off: the plain SDL presentation, exactly as before.
	if (g_shader == 0 && filter.kind == FILTER_NONE)
		return false;
	if (!renderer || !tex || viewport.w <= 0 || viewport.h <= 0)
		return false;

	// Run SDL's queued commands (the backbuffer clear, the target switch), so
	// the window framebuffer is bound, and make SDL's GL context current.
	SDL_RenderFlush(renderer);
	float texW = 1.0f, texH = 1.0f;
	if (SDL_GL_BindTexture(tex, &texW, &texH) != 0)
		return false;
	if (!loadGL() || !build(mode.program)) {
		SDL_GL_UnbindTexture(tex);
		return false;
	}
	Program& p = g_programs[mode.program];
	int texPixW = 0, texPixH = 0;
	Uint32 texFormat = 0;
	SDL_QueryTexture(tex, &texFormat, nullptr, &texPixW, &texPixH);
	// Channel order of the GL texture. The game buffer is ABGR8888 (GL's own
	// RGBA order); SDL stores the ARGB/RGB formats red/blue swapped. Any other
	// format is not native to SDL's GLES2 renderer and is backed by a hidden
	// ARGB8888 texture, which SDL_QueryTexture does not report, so it is
	// swapped too.
	bool swapRB = !(texFormat == SDL_PIXELFORMAT_ABGR8888 || texFormat == SDL_PIXELFORMAT_BGR888);
	int outW = 0, outH = 0;
	SDL_GetRendererOutputSize(renderer, &outW, &outH);

	// Save the GL state this draw touches.
	GLint prevProgram = 0, prevArrayBuffer = 0, prevActive = 0, prevTex = 0;
	GLint prevViewport[4] = {0, 0, 0, 0};
	GLint prevMin = GL_NEAREST, prevMag = GL_NEAREST;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevArrayBuffer);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActive);
	glGetIntegerv(GL_VIEWPORT, prevViewport);
	GLboolean prevBlend = glIsEnabled(GL_BLEND);
	GLboolean prevScissor = glIsEnabled(GL_SCISSOR_TEST);
	AttribState attribs[ATTR_SAVED];
	for (GLuint i = 0; i < ATTR_SAVED; i++) {
		AttribState& a = attribs[i];
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &a.enabled);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &a.size);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &a.type);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &a.normalized);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &a.stride);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &a.buffer);
		glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &a.pointer);
	}

	// The texture SDL just bound, on whichever unit is active: sample it from
	// unit 0 (and put unit 0's binding back afterwards).
	GLint sdlTex = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &sdlTex);
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTex);
	glBindTexture(GL_TEXTURE_2D, sdlTex);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &prevMin);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &prevMag);
	// The shaders do their own pixel-perfect scaling from LINEAR taps.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (!g_vbo) {
		// Clip-space position, then texcoord with v = 0 at the top row (SDL's
		// convention for render targets as well as ordinary textures).
		const GLfloat quad[16] = {
			-1.0f, -1.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 1.0f, 0.0f,
		};
		glGenBuffers(1, &g_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	}

	glUseProgram(p.id);
	// SDL viewports are top-left origin; GL's is bottom-left.
	glViewport(viewport.x, outH - viewport.y - viewport.h, viewport.w, viewport.h);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	const GLfloat identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	if (p.uMVP >= 0) glUniformMatrix4fv(p.uMVP, 1, GL_FALSE, identity);
	if (p.uOutputSize >= 0) glUniform2f(p.uOutputSize, (float)viewport.w, (float)viewport.h);
	// GLES2 textures are never padded (texW == texH == 1), so the texture
	// and the game picture are the same size.
	(void)texW;
	(void)texH;
	if (p.uTextureSize >= 0) glUniform2f(p.uTextureSize, (float)texPixW, (float)texPixH);
	if (p.uInputSize >= 0) glUniform2f(p.uInputSize, (float)texPixW, (float)texPixH);
	if (p.uFrameCount >= 0) glUniform1i(p.uFrameCount, g_frameCount);
	if (p.uFrameDirection >= 0) glUniform1i(p.uFrameDirection, 1);
	if (p.uTexture >= 0) glUniform1i(p.uTexture, 0);
	for (size_t i = 0; i < p.params.size(); i++)
		if (p.params[i].loc >= 0)
			glUniform1f(p.params[i].loc, param_value(g_shader, p.params[i].name, overrideValue(mode.overrides, p.params[i].name, p.params[i].def)));
	if (p.uSwapRB >= 0)
		glUniform1i(p.uSwapRB, swapRB ? 1 : 0);
	if (p.uCorrect >= 0)
		glUniform1i(p.uCorrect, mode.correction);
	if (p.uFilterMode >= 0)
		glUniform1i(p.uFilterMode, filter.kind);
	if (p.uFilterPal >= 0 && filter.kind == FILTER_PALETTE) {
		GLfloat pal[12];
		for (int i = 0; i < 4; i++) {
			pal[i * 3 + 0] = ((filter.pal[i] >> 16) & 0xFF) / 255.0f;
			pal[i * 3 + 1] = ((filter.pal[i] >> 8) & 0xFF) / 255.0f;
			pal[i * 3 + 2] = (filter.pal[i] & 0xFF) / 255.0f;
		}
		glUniform3fv(p.uFilterPal, 4, pal);
	}

	glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
	glEnableVertexAttribArray(ATTR_VERTEX);
	glEnableVertexAttribArray(ATTR_TEXCOORD);
	glDisableVertexAttribArray(ATTR_COLOR);
	glVertexAttribPointer(ATTR_VERTEX, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	g_frameCount++;

	// Restore.
	for (GLuint i = 0; i < ATTR_SAVED; i++) {
		const AttribState& a = attribs[i];
		glBindBuffer(GL_ARRAY_BUFFER, a.buffer);
		if (a.size > 0)
			glVertexAttribPointer(i, a.size, a.type, a.normalized ? GL_TRUE : GL_FALSE, a.stride, a.pointer);
		if (a.enabled)
			glEnableVertexAttribArray(i);
		else
			glDisableVertexAttribArray(i);
	}
	glBindBuffer(GL_ARRAY_BUFFER, prevArrayBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, prevMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, prevMag);
	glBindTexture(GL_TEXTURE_2D, prevTex);
	glActiveTexture(prevActive);
	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
	if (prevBlend) glEnable(GL_BLEND);
	if (prevScissor) glEnable(GL_SCISSOR_TEST);
	glUseProgram(prevProgram);
	SDL_GL_UnbindTexture(tex);
	return true;
}


extern "C" bool rt_postfx_draw(SDL_Renderer* renderer, SDL_Texture* tex, const SDL_Rect* vp) {
	SDL_RendererInfo info{};
	if (SDL_GetRendererInfo(renderer, &info) != 0 || !info.name || std::strcmp(info.name, "opengles2") != 0)
		return false;
	return postfx_Draw(renderer, tex, *vp);
}

// Wait for the GPU after presenting (input latency: at most one frame queued).
extern "C" void rt_postfx_finish(SDL_Renderer* r) {
	static const bool on = [] {
		const char* v = std::getenv("RECOMP_GLFINISH");
		return v ? std::atoi(v) != 0 : true;
	}();
	static PFNGLFINISHPROC finish = nullptr;
	if (!on || !r) return;
	if (!finish) finish = (PFNGLFINISHPROC)SDL_GL_GetProcAddress("glFinish");
	if (finish) finish();
}
#endif  // __SWITCH__
