cmake_minimum_required(VERSION 3.2)
project(Slug)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(GCC_COVERAGE_COMPILE_FLAGS "-std=c++17" "-ggdb")
set(GCC_COVERAGE_LINK_FLAGS "-lglfw" "-lvulkan" "-ldl" "-lpthread" "-lX11" "-lXxf86vm" "-lXrandr" "-lXi")


set(SOURCE_FILES 
  "main.cpp"
  "Engine.cpp"
  "Window.cpp"

  "common/Allocator.cpp"
  "common/VulkanErrors.cpp"
  "common/File.cpp"

  "include/tlsf.cpp"
  "include/vk_mem_alloc.cpp"
)
add_executable(Slug ${SOURCE_FILES})

target_compile_options(Slug PRIVATE "${GCC_COVERAGE_COMPILE_FLAGS}")
target_link_libraries(Slug PRIVATE "${GCC_COVERAGE_LINK_FLAGS}")

target_include_directories(Slug PUBLIC 
  "${PROJECT_BINARY_DIR}" 
  "common" 
  "include"
)
