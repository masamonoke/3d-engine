cmake_minimum_required(VERSION 3.22)
project(3d_model_viewer)

set(CMAKE_CXX_STANDARD 20)

set(GLFW "lib/glfw-3.3.8")

include_directories(/Users/masamonoke/lib/vulkan/macOS/include)
set(Vulkan_LIBRARY /Users/masamonoke/lib/vulkan/macOS/lib)
set(Vulkan_INCLUDE_DIR /Users/masamonoke/lib/vulkan/macOS/include)
find_package(Vulkan REQUIRED)

add_subdirectory(${GLFW})
include_directories(${GLFW}/include)

include_directories(include)

add_subdirectory(src)

