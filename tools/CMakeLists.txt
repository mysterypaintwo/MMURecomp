add_executable(mmu_bake bake_assets.cpp)
target_link_libraries(mmu_bake PRIVATE mmu_core)

add_executable(mmu_gameflow_runtime_selftest gameflow_runtime_selftest.cpp)
target_link_libraries(mmu_gameflow_runtime_selftest PRIVATE mmu_core)

if(NOT MSVC)
  target_compile_options(mmu_bake PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(mmu_gameflow_runtime_selftest PRIVATE -Wall -Wextra -Wpedantic)
endif()
