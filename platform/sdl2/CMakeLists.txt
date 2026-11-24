if(NOT CMAKE_CROSSCOMPILING)
  find_package(SDL2 REQUIRED CONFIG)
endif()

set(MMU_IMGUI_SOURCES
  ${MMU_IMGUI_DIR}/imgui.cpp
  ${MMU_IMGUI_DIR}/imgui_demo.cpp
  ${MMU_IMGUI_DIR}/imgui_draw.cpp
  ${MMU_IMGUI_DIR}/imgui_tables.cpp
  ${MMU_IMGUI_DIR}/imgui_widgets.cpp
  ${MMU_IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
  ${MMU_IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

add_library(mmu_imgui STATIC ${MMU_IMGUI_SOURCES})
target_include_directories(mmu_imgui PUBLIC
  ${MMU_IMGUI_DIR}
  ${MMU_IMGUI_DIR}/backends
)
if(CMAKE_CROSSCOMPILING)
  target_include_directories(mmu_imgui PRIVATE ${MMU_SDL2_INCLUDE_DIRS})
  # devkitPro newlib has no execvp/waitpid; ImGui doesn't need shell launch.
  target_compile_definitions(mmu_imgui PUBLIC IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS)
else()
  target_link_libraries(mmu_imgui PRIVATE SDL2::SDL2)
endif()
if(NOT MSVC)
  target_compile_options(mmu_imgui PRIVATE -Wno-unknown-warning-option -Wno-deprecated-declarations)
endif()

set(MMU_STB_VORBIS_IMPL "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/stb_vorbis_impl.cpp")
add_executable(mmu_sdl2
  src/main.cpp
  src/AudioEngine.cpp
  ${MMU_STB_VORBIS_IMPL}
)
target_include_directories(mmu_sdl2 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party
)
target_link_libraries(mmu_sdl2 PRIVATE mmu_core mmu_imgui)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set_source_files_properties(${MMU_STB_VORBIS_IMPL} PROPERTIES COMPILE_OPTIONS "-Wno-everything")
else()
  set_source_files_properties(${MMU_STB_VORBIS_IMPL} PROPERTIES COMPILE_OPTIONS "-w")
endif()

if(CMAKE_CROSSCOMPILING)
  target_include_directories(mmu_sdl2 PRIVATE ${MMU_SDL2_INCLUDE_DIRS})
  target_link_directories(mmu_sdl2 PRIVATE ${MMU_SDL2_LIBRARY_DIRS})
  target_link_libraries(mmu_sdl2 PRIVATE ${MMU_SDL2_LIBRARIES})
else()
  target_link_libraries(mmu_sdl2 PRIVATE SDL2::SDL2)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(mmu_sdl2 PRIVATE SDL2::SDL2main)
  endif()
endif()

if(CMAKE_CROSSCOMPILING)
  find_program(MMU_ELF2NRO elf2nro HINTS ${DEVKITPRO}/tools/bin)
  find_program(MMU_NACPTOOL nacptool HINTS ${DEVKITPRO}/tools/bin)
  if(MMU_ELF2NRO AND MMU_NACPTOOL)
    set(MMU_NACP "${CMAKE_CURRENT_BINARY_DIR}/mmu_recomp.nacp")
    add_custom_command(OUTPUT "${MMU_NACP}"
      COMMAND ${MMU_NACPTOOL} --create "MMU Recomp" "MMU contributors" "0.1.0" "${MMU_NACP}"
      VERBATIM)
    add_custom_target(mmu_nacp DEPENDS "${MMU_NACP}")
    add_dependencies(mmu_sdl2 mmu_nacp)
    set(MMU_NRO_ARGS --icon=${LIBOUNCE}/default_icon.jpg --nacp=${MMU_NACP})
    if(MMU_ROMFS_DIR)
      if(IS_DIRECTORY "${MMU_ROMFS_DIR}")
        list(APPEND MMU_NRO_ARGS --romfsdir=${MMU_ROMFS_DIR})
      else()
        message(WARNING
          "MMU_ROMFS_DIR='${MMU_ROMFS_DIR}' is not a directory; building .nro without romfs. "
          "Point it at a directory containing assets.pak, or leave it unset.")
      endif()
    endif()
    add_custom_command(TARGET mmu_sdl2 POST_BUILD
      COMMAND ${MMU_ELF2NRO} $<TARGET_FILE:mmu_sdl2>
              $<TARGET_FILE_DIR:mmu_sdl2>/mmu_sdl2.nro ${MMU_NRO_ARGS}
      VERBATIM)
  endif()
endif()

target_compile_definitions(mmu_sdl2 PRIVATE MMU_ASSET_DIR="${MMU_ASSET_DIR}")

# Convenience target: bake straight into the directory that contains the
# executable, which is where the host runtime looks by default.
if(NOT CMAKE_CROSSCOMPILING AND TARGET mmu_bake)
  add_custom_target(bake_assets
    COMMAND $<TARGET_FILE:mmu_bake> $<TARGET_FILE_DIR:mmu_sdl2>/assets.pak "${MMU_ASSET_DIR}"
    DEPENDS mmu_bake
    COMMENT "Baking assets.pak next to mmu_sdl2")
endif()
