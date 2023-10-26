add_executable(model_viewer main.cpp window.cpp app.cpp)

target_compile_options(model_viewer PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_link_libraries(model_viewer glfw ${GLFW_LIBRARIES})
