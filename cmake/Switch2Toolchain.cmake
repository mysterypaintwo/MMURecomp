# Switch 2 (libounce + devkitPro portlibs) toolchain.
#   source $DEVKITPRO/switch2vars.sh
#   cmake -S . -B build-switch2 -DCMAKE_TOOLCHAIN_FILE=cmake/Switch2Toolchain.cmake
#   cmake --build build-switch2

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

if(NOT DEFINED ENV{DEVKITPRO})
  message(FATAL_ERROR "DEVKITPRO is not set. Source \$DEVKITPRO/switch2vars.sh first.")
endif()
set(DEVKITPRO "$ENV{DEVKITPRO}" CACHE PATH "devkitPro root")
set(DEVKITA64 "${DEVKITPRO}/devkitA64")
set(LIBOUNCE "${DEVKITPRO}/libounce")
set(PORTLIBS "${DEVKITPRO}/portlibs/switch")
set(PORTLIBS2 "${DEVKITPRO}/portlibs/switch2")

set(CMAKE_C_COMPILER   "${DEVKITA64}/bin/aarch64-none-elf-gcc")
set(CMAKE_CXX_COMPILER "${DEVKITA64}/bin/aarch64-none-elf-g++")
set(CMAKE_AR           "${DEVKITA64}/bin/aarch64-none-elf-ar")
set(CMAKE_RANLIB       "${DEVKITA64}/bin/aarch64-none-elf-ranlib")
set(CMAKE_OBJCOPY      "${DEVKITA64}/bin/aarch64-none-elf-objcopy")

set(MMU_SWITCH2_CPU -mcpu=cortex-a78c+crypto)
add_compile_options(${MMU_SWITCH2_CPU} -mtp=soft -fPIE -ffunction-sections -fdata-sections
                    -D__SWITCH__ -D__SWITCH2__)
add_link_options(-specs=${LIBOUNCE}/switch.specs ${MMU_SWITCH2_CPU} -mtp=soft -fPIE
                 -Wl,--gc-sections)

include_directories(${PORTLIBS}/include ${PORTLIBS2}/include ${LIBOUNCE}/include)
# switch2/lib MUST come before switch/lib: it holds OunceVK's tiny libEGL.a
# shim. Linking switch/lib's full libEGL.a instead pulls in unresolved nouveau
# symbols (nouveau_pushbuf_space, ...). Keep this order even without LDFLAGS.
link_directories(${PORTLIBS2}/lib ${PORTLIBS}/lib ${LIBOUNCE}/lib)

# SDL2 from the Switch 1 portlibs works unchanged under libounce. Use library
# names (not absolute paths) so the -L order above selects the EGL shim.
set(MMU_SDL2_INCLUDE_DIRS "${PORTLIBS}/include/SDL2;${PORTLIBS}/include" CACHE STRING "" FORCE)
set(MMU_SDL2_LIBRARY_DIRS "${PORTLIBS2}/lib;${PORTLIBS}/lib;${LIBOUNCE}/lib" CACHE STRING "" FORCE)
# libounce ships an empty libnx.a compatibility stub; real symbols live in
# libounce.a.
set(MMU_SDL2_LIBRARIES "SDL2;ounce;EGL" CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH "${DEVKITA64}" "${PORTLIBS}" "${PORTLIBS2}" "${LIBOUNCE}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
