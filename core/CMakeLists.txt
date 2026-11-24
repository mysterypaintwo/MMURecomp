add_library(mmu_core STATIC
  src/ProtoReader.cpp
  src/AssetDatabase.cpp
  src/CutsceneRuntime.cpp
  src/PackedArchive.cpp
  src/DebugConsole.cpp
  src/GameFlowRuntime.cpp
  src/KeyedFlagRuntime.cpp
  src/StateDispatcher.cpp
  src/World.cpp
  src/LevelArt.cpp
  src/PlayerState.cpp
  src/EnemyState.cpp
)

target_include_directories(mmu_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(mmu_core PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(mmu_core PRIVATE /W4)
else()
  target_compile_options(mmu_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
