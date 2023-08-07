cmake_minimum_required(VERSION 3.2) 
project(Slug)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_DEBUG ON)
if(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE DEBUG) 
else()
    set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE) 
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE) 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(GCC_COVERAGE_COMPILE_FLAGS 
    "-std=c++17" 
    "-g"
)
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

set(SOURCE_FILES
    "Engine.cpp"
    "FeaturesExtensions.cpp"

    "common/Window.cpp"
    "common/Allocator.cpp"
    "common/VulkanErrors.cpp"
    "common/File.cpp"
    "common/Camera.cpp"
    "common/Clock.cpp"
    "common/String.cpp"
    "common/glTF.cpp"
    "common/Image.cpp"
    "common/Spirv.cpp"
    "common/Spv.cpp"

    "external/tlsf.cpp"
    "external/vk_mem_alloc.cpp"
    "external/stb_image.cpp"
)
add_executable(Slug "main.cpp" ${SOURCE_FILES})
target_include_directories(Slug PUBLIC 
    "${PROJECT_BINARY_DIR}" 
    "common" 
    "external"
)
target_compile_options(Slug PRIVATE "${GCC_COVERAGE_COMPILE_FLAGS}") 
target_link_libraries(Slug PRIVATE "${GCC_COVERAGE_LINK_FLAGS}") 

option(BUILD_TESTS ON)
if(BUILD_TESTS)
    set(TEST_FILES 
        test/test.cpp
        test/spv_test.cpp
    )
    add_executable(TestAll 
        test/test_entrypoint.cpp 
        ${TEST_FILES} 
        ${SOURCE_FILES}
    )
    target_include_directories(TestAll PUBLIC 
        "${PROJECT_BINARY_DIR}" 
        "common" 
        "external"
        "test"
    )
    target_compile_options(TestAll PRIVATE "${GCC_COVERAGE_COMPILE_FLAGS}") 
    target_link_libraries(TestAll PRIVATE "${GCC_COVERAGE_LINK_FLAGS}") 
endif()
