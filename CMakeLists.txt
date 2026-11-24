cmake_minimum_required(VERSION 3.20)
project(mmu_recomp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MMU_BUILD_SDL2 "Build the SDL2 front-end" ON)

set(MMU_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../imgui" CACHE PATH "Path to Dear ImGui source tree")
set(MMU_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MegaManUnlimited/Assets" CACHE PATH "Path to original .mmu assets")
set(MMU_ROMFS_DIR "" CACHE PATH "Directory embedded as Switch 2 romfs (should contain assets.pak)")
if(CMAKE_CROSSCOMPILING AND MMU_ROMFS_DIR AND IS_DIRECTORY "${MMU_ROMFS_DIR}")
  set(MMU_ASSET_DIR "romfs:/" CACHE PATH "Asset directory at runtime" FORCE)
endif()

add_subdirectory(core)

# The baker runs on the host; it cannot be compiled by the console toolchain.
if(NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(tools)
endif()

if(MMU_BUILD_SDL2)
  add_subdirectory(platform/sdl2)
endif()
