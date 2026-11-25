cmake_minimum_required(VERSION 3.20)
project(mmu_recomp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MMU_BUILD_SDL2 "Build the SDL2 front-end" ON)

# Switch 2 (devkitPro toolchain, "Generic" system). Other cross builds (MinGW
# for Windows) are ordinary desktop builds.
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Generic")
  set(MMU_SWITCH ON)
else()
  set(MMU_SWITCH OFF)
  # Desktop SDL2. A static-only SDL2 (release builds) has no SDL2::SDL2 target.
  find_package(SDL2 REQUIRED CONFIG)
  if(NOT TARGET SDL2::SDL2 AND TARGET SDL2::SDL2-static)
    add_library(SDL2::SDL2 INTERFACE IMPORTED GLOBAL)
    target_link_libraries(SDL2::SDL2 INTERFACE SDL2::SDL2-static)
  endif()
endif()
option(MMU_RECOMP "Build the static recompilation of MMU.exe (needs MMU_GAME_DIR)" ON)

set(MMU_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../imgui" CACHE PATH "Path to Dear ImGui source tree")
set(MMU_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MegaManUnlimited/Assets" CACHE PATH "Path to original .mmu assets")
set(MMU_ROMFS_DIR "" CACHE PATH "Directory embedded as Switch 2 romfs (advanced; prefer MMU_PAK)")
set(MMU_PAK "" CACHE FILEPATH "assets.pak to embed in the Switch 2 .nro (staged alone into <build>/romfs)")
if(MMU_SWITCH AND ((MMU_ROMFS_DIR AND IS_DIRECTORY "${MMU_ROMFS_DIR}") OR
                             (MMU_PAK AND EXISTS "${MMU_PAK}")))
  set(MMU_ASSET_DIR "romfs:/" CACHE PATH "Asset directory at runtime" FORCE)
endif()

get_filename_component(_mmu_default_game_dir "${MMU_ASSET_DIR}" DIRECTORY)
set(MMU_GAME_DIR "${_mmu_default_game_dir}" CACHE PATH "Original game folder (MMU.exe, msvcr90.dll, msvcp90.dll, Assets/)")

add_subdirectory(core)
if(MMU_RECOMP AND EXISTS "${MMU_GAME_DIR}/MMU.exe")
  add_subdirectory(recomp)
elseif(MMU_RECOMP)
  message(STATUS "MMU_GAME_DIR has no MMU.exe; skipping the recompiled build")
endif()

# The baker runs on the host; it cannot be compiled by the console toolchain.
if(NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(tools)
endif()

if(MMU_BUILD_SDL2)
  add_subdirectory(platform/sdl2)
endif()
