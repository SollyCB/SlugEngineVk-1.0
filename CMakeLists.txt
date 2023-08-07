cmake_minimum_required(VERSION 3.2) 
project(Slug)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE) 
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE) 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(GCC_COVERAGE_LINK_FLAGS 
    "-lglfw"
    "-lvulkan"
    "-ldl"
    "-lpthread"
    "-lX11"
    "-lXxf86vm"
    "-lXrandr"
    "-lXi"
)

option(BUILD_DEBUG ON)
option(BUILD_TESTS ON)

if(BUILD_DEBUG)
    message("Building DEBUG...")
    set(CMAKE_BUILD_TYPE DEBUG)
    add_compile_options(
        -std=c++17
        -g
    )
    add_compile_definitions(DEBUG=true)
else()
    message("Building RELEASE...")
    set(CMAKE_BUILD_TYPE RELEASE)
    add_compile_options(
        -std=c++17
        -O3
    )
    add_compile_definitions(DEBUG=false)
endif()

set(SOURCE_FILES
    "Engine.cpp"
    "FeaturesExtensions.cpp"

    "external/vk_mem_alloc.cpp"
)
add_executable(Slug "main.cpp" ${SOURCE_FILES})
target_include_directories(Slug PUBLIC 
    "${PROJECT_BINARY_DIR}" 
    "common" 
    "common/external"
    "external"
)
target_compile_options(Slug PUBLIC "${GCC_COVERAGE_COMPILE_FLAGS}") 
add_subdirectory(common)
target_link_libraries(Slug PUBLIC "${GCC_COVERAGE_LINK_FLAGS}" SlugVk-lib-common) 

if(BUILD_TESTS)
    message("Building Tests...")
    add_executable(TestTop 
        test/test_entrypoint.cpp 
        test/test.cpp
    )
    target_include_directories(TestTop PUBLIC 
        "${PROJECT_BINARY_DIR}" 
        "common" 
        "common/external"
        "external"
        "test"
    )
    target_compile_options(TestTop PRIVATE "${GCC_COVERAGE_COMPILE_FLAGS}") 
    target_link_libraries(TestTop PRIVATE "${GCC_COVERAGE_LINK_FLAGS}" SlugVk-lib-common) 
endif()
