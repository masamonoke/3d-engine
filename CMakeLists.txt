include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)

cmake_minimum_required(VERSION 3.22)

set(NAME 3d_model_viewer)
project(${NAME})

set(CMAKE_CXX_STANDARD 20)


include_directories(${GLFW_INCLUDE})

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp src/window.cpp src/app.cpp src/pipeline.cpp src/engine_device.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})

find_package(glfw3 3.3 REQUIRED)
set(GLFW_LIB glfw)
message(STATUS "Found GLFW")

target_link_libraries(${PROJECT_NAME} ${GLFW_LIB} vulkan)
