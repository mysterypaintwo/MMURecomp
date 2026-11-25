# Static recompilation of MMU.exe (+ msvcr90/msvcp90). The C sources are
# generated at build time from the user's own game files by
# tools/recomp/lift.py and never leave the build directory.
# The lifter needs capstone (pip3 install -r tools/recomp/requirements.txt).
find_program(MMU_PYTHON NAMES python3 python REQUIRED)

set(MMU_RECOMP_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(MMU_RECOMP_FILES 48)
set(MMU_RECOMP_GEN_SOURCES
  "${MMU_RECOMP_GEN_DIR}/recomp_table.c"
  "${MMU_RECOMP_GEN_DIR}/recomp_import_stubs.c")
math(EXPR _last "${MMU_RECOMP_FILES} - 1")
foreach(i RANGE 0 ${_last})
  string(LENGTH "${i}" _len)
  if(_len EQUAL 1)
    set(_n "00${i}")
  else()
    set(_n "0${i}")
  endif()
  list(APPEND MMU_RECOMP_GEN_SOURCES "${MMU_RECOMP_GEN_DIR}/recomp_${_n}.c")
endforeach()

add_custom_command(
  OUTPUT ${MMU_RECOMP_GEN_SOURCES} "${MMU_RECOMP_GEN_DIR}/recomp_decls.h"
  COMMAND ${MMU_PYTHON} "${CMAKE_SOURCE_DIR}/tools/recomp/lift.py"
          "${MMU_GAME_DIR}/MMU.exe" "${MMU_RECOMP_GEN_DIR}" --files ${MMU_RECOMP_FILES}
  DEPENDS "${CMAKE_SOURCE_DIR}/tools/recomp/lift.py" "${MMU_GAME_DIR}/MMU.exe"
          "${CMAKE_SOURCE_DIR}/docs/function_inventory.txt"
  COMMENT "Recompiling MMU.exe -> C (tools/recomp/lift.py)"
  VERBATIM)

add_library(mmu_recomp_gen STATIC ${MMU_RECOMP_GEN_SOURCES})
target_include_directories(mmu_recomp_gen PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include" "${MMU_RECOMP_GEN_DIR}")
set_target_properties(mmu_recomp_gen PROPERTIES C_STANDARD 11)
if(MSVC)
  target_compile_options(mmu_recomp_gen PRIVATE /w /O2)
else()
  # Generated code: silence warnings, keep x86 integer wrap-around semantics.
  target_compile_options(mmu_recomp_gen PRIVATE -w -O2 -fwrapv -fno-strict-aliasing)
endif()

# OBJECT library: every shim object is linked, so strong shims always
# override the generated weak fallbacks in recomp_import_stubs.c.
# Screen shaders (recomp/shaders, MIT) embedded as C strings for postfx.cpp.
set(MMU_SHADER_HEADER "${CMAKE_CURRENT_BINARY_DIR}/postfx_shaders.h")
file(WRITE "${MMU_SHADER_HEADER}.tmp" "// Generated from recomp/shaders by CMake.\n#pragma once\n")
foreach(pair "kShaderPixel=pixel-perfect" "kShaderCrt=crt-perfect" "kShaderLcd=lcd-perfect" "kShaderDmg=dmg-perfect")
  string(REPLACE "=" ";" pair "${pair}")
  list(GET pair 0 var)
  list(GET pair 1 file)
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${file}.glsl" text)
  file(APPEND "${MMU_SHADER_HEADER}.tmp" "static const char* const ${var} = R\"mmuglsl(${text})mmuglsl\";\n")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${file}.glsl")
endforeach()
file(COPY_FILE "${MMU_SHADER_HEADER}.tmp" "${MMU_SHADER_HEADER}" ONLY_IF_DIFFERENT)

# The game's 8x8 sprite font (Assets/misc.mmu) for the port's own messages.
set(MMU_FONT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mmu_font.h")
add_custom_command(OUTPUT "${MMU_FONT_HEADER}"
  COMMAND ${MMU_PYTHON} "${CMAKE_SOURCE_DIR}/tools/recomp/extract_font.py" "${MMU_GAME_DIR}" "${MMU_FONT_HEADER}"
  DEPENDS "${CMAKE_SOURCE_DIR}/tools/recomp/extract_font.py"
  COMMENT "Extracting the game font (tools/recomp/extract_font.py)"
  VERBATIM)
add_custom_target(mmu_recomp_font DEPENDS "${MMU_FONT_HEADER}")

add_library(mmu_recomp_rt OBJECT
  src/runtime.c
  src/heap.c
  src/imports_kernel32.cpp
  src/imports_sdl.cpp
  src/imports_debug.cpp
  src/log.cpp
  src/patches.cpp
  src/postfx.cpp
  src/port_menu.cpp
  src/native_text.cpp
  src/missing_game.cpp)
target_include_directories(mmu_recomp_rt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${MMU_RECOMP_GEN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_features(mmu_recomp_rt PUBLIC cxx_std_17)
add_dependencies(mmu_recomp_rt mmu_recomp_gen mmu_recomp_font)
# Port menu: Dear ImGui (mmu_imgui from platform/sdl2, MMU_IMGUI_DIR).
target_link_libraries(mmu_recomp_rt PUBLIC mmu_imgui)
if(MMU_SWITCH)
  target_include_directories(mmu_recomp_rt PUBLIC ${MMU_SDL2_INCLUDE_DIRS})
  target_link_directories(mmu_recomp_rt PUBLIC ${MMU_SDL2_LIBRARY_DIRS})
  target_link_libraries(mmu_recomp_rt PUBLIC ${MMU_SDL2_LIBRARIES})
else()
  target_link_libraries(mmu_recomp_rt PUBLIC SDL2::SDL2)
endif()

add_executable(mmu_recomp src/main.cpp)
if(MINGW)
  # COFF never pulls weak definitions (recomp_import_stubs.c) out of an archive.
  target_link_libraries(mmu_recomp PRIVATE mmu_recomp_rt "$<LINK_LIBRARY:WHOLE_ARCHIVE,mmu_recomp_gen>")
  target_link_options(mmu_recomp PRIVATE -Wl,--allow-multiple-definition)
else()
  target_link_libraries(mmu_recomp PRIVATE mmu_recomp_rt mmu_recomp_gen mmu_recomp_rt)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Release binaries: only glibc and system libraries at run time.
  target_link_options(mmu_recomp PRIVATE -static-libgcc -static-libstdc++)
endif()
if(WIN32)
  # A window, no console; SDL2main provides WinMain. MinGW: no libgcc/libstdc++ DLLs.
  set_target_properties(mmu_recomp PROPERTIES WIN32_EXECUTABLE ON)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(mmu_recomp PRIVATE SDL2::SDL2main)
  endif()
  if(MINGW)
    target_link_options(mmu_recomp PRIVATE -static)
  endif()
endif()

# Shared library of the generated code + runtime for tools/recomp/difftest.py.
if(NOT CMAKE_CROSSCOMPILING)
  add_library(mmu_recomp_testlib SHARED $<TARGET_OBJECTS:mmu_recomp_rt>)
  target_link_libraries(mmu_recomp_testlib PRIVATE mmu_recomp_gen SDL2::SDL2)
  set_target_properties(mmu_recomp_testlib PROPERTIES EXCLUDE_FROM_ALL ON POSITION_INDEPENDENT_CODE ON)
  set_target_properties(mmu_recomp_gen PROPERTIES POSITION_INDEPENDENT_CODE ON)
  set_target_properties(mmu_recomp_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Switch 2: package mmu_recomp as an .nro. Game files are read from
# sdmc:/switch/mmu_recomp/game at run time; MMU_RECOMP_ROMFS_DIR optionally
# embeds a directory that contains game/ (read-only fallback).
if(MMU_SWITCH)
  find_program(MMU_ELF2NRO elf2nro HINTS ${DEVKITPRO}/tools/bin)
  find_program(MMU_NACPTOOL nacptool HINTS ${DEVKITPRO}/tools/bin)
endif()
if(MMU_SWITCH AND MMU_ELF2NRO AND MMU_NACPTOOL)
  set(MMU_RECOMP_NACP "${CMAKE_CURRENT_BINARY_DIR}/mmu_recomp.nacp")
  add_custom_command(OUTPUT "${MMU_RECOMP_NACP}"
    COMMAND ${MMU_NACPTOOL} --create "Mega Man Unlimited" "MegaPhilX (Switch 2 port by mysterypaint)" "1.0" "${MMU_RECOMP_NACP}"
    VERBATIM)
  # Icon: the 1UP Mega Man head sprite x16 (tools/recomp/extract_icon.py, needs Pillow + protobuf),
  # else libounce's default.
  set(MMU_RECOMP_ICON "${CMAKE_CURRENT_BINARY_DIR}/mmu_icon.jpg")
  add_custom_command(OUTPUT "${MMU_RECOMP_ICON}"
    COMMAND sh -c "\"$0\" \"$1\" \"$2\" \"$3\" || cp \"$4\" \"$3\""
            ${MMU_PYTHON} "${CMAKE_SOURCE_DIR}/tools/recomp/extract_icon.py" "${MMU_GAME_DIR}"
            "${MMU_RECOMP_ICON}" "${LIBOUNCE}/default_icon.jpg"
    DEPENDS "${CMAKE_SOURCE_DIR}/tools/recomp/extract_icon.py" "${MMU_GAME_DIR}/Assets/gameobjects.mmu"
    VERBATIM)
  add_custom_target(mmu_recomp_nacp DEPENDS "${MMU_RECOMP_NACP}" "${MMU_RECOMP_ICON}")
  add_dependencies(mmu_recomp mmu_recomp_nacp)
  set(MMU_RECOMP_NRO_ARGS --icon=${MMU_RECOMP_ICON} --nacp=${MMU_RECOMP_NACP})
  set(MMU_RECOMP_ROMFS_DIR "" CACHE PATH "Optional romfs root for mmu_recomp.nro (containing game/)")
  if(MMU_RECOMP_ROMFS_DIR AND IS_DIRECTORY "${MMU_RECOMP_ROMFS_DIR}")
    list(APPEND MMU_RECOMP_NRO_ARGS --romfsdir=${MMU_RECOMP_ROMFS_DIR})
  endif()
  add_custom_command(TARGET mmu_recomp POST_BUILD
    COMMAND ${MMU_ELF2NRO} $<TARGET_FILE:mmu_recomp> $<TARGET_FILE_DIR:mmu_recomp>/mmu_recomp.nro ${MMU_RECOMP_NRO_ARGS}
    VERBATIM)
endif()
