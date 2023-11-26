include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)

cmake_minimum_required(VERSION 3.22)

set(NAME 3d_model_viewer)
project(${NAME})

set(CMAKE_CXX_STANDARD 20)

include_directories(${GLFW_INCLUDE})

include_directories(include)

add_custom_command(
	OUTPUT shader/build/simple_shader.frag.spv shader/build/simple_shader.vert.spv
	COMMAND "./compile.sh"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_custom_target(
	BUILD_SHADERS ALL
	DEPENDS shader/build/simple_shader.frag.spv shader/build/simple_shader.vert.spv
)

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/window.cpp
	src/app.cpp
	src/pipeline.cpp
	src/engine_device.cpp
	src/swap_chain.cpp
	src/model.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -fsanitize=address)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})

find_package(glfw3 3.3 REQUIRED)
message(STATUS "Found GLFW")

find_package(glm REQUIRED)
message(STATUS "Found glm")

target_link_libraries(${PROJECT_NAME} glfw vulkan glm::glm)
