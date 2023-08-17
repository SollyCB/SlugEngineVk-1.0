cmake_minimum_required(VERSION 3.2) 
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_PARALLEL_LEVEL 4)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Slug)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE) 
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE) 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

option(BUILD_DEBUG ON)
option(BUILD_TESTS ON)

if(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    add_compile_definitions(DEBUG=true)
	add_compile_definitions(V_LAYERS=true)
    set(COMPILE_FLAGS_SLUGVK_TOP "-std=c++17")
else()
    message("Building RELEASE...")
    set(CMAKE_BUILD_TYPE Release)
    add_compile_definitions(DEBUG=false)
	add_compile_definitions(V_LAYERS=false)
    set(COMPILE_FLAGS_SLUGVK_TOP "-std=c++17")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS_SLUGVK_TOP}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS_SLUGVK_TOP}")
set(LINK_FLAGS_SLUGVK_TOP 
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
	"VulkanErrors.cpp"

    "external/vk_mem_alloc.cpp"
)
add_executable(Slug "main.cpp" ${SOURCE_FILES})
target_include_directories(Slug PUBLIC 
    "${PROJECT_BINARY_DIR}" 
    "common" 
    "common/external"
    "external"
)
target_compile_options(Slug PUBLIC "${COMPILE_FLAGS_SLUGVK_TOP}") 
add_subdirectory(common)
target_link_libraries(Slug PUBLIC "${LINK_FLAGS_SLUGVK_TOP}" SlugVk-lib-common) 

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
    target_compile_options(TestTop PRIVATE "${COMPILE_FLAGS_SLUGVK_TOP}") 
    target_link_libraries(TestTop PRIVATE "${LINK_FLAGS_SLUGVK_TOP}" SlugVk-lib-common) 
endif()
