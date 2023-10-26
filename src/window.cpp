#include "window.hpp"

namespace engine {
	Window::Window(int w, int h, std::string title) : width_(w), height_(h), title_(title) {
		this->initWindow();
	}

	Window::~Window() {
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	void Window::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window_ = glfwCreateWindow(
				width_,
				height_,
				title_.c_str(),
				nullptr,
				nullptr);

	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(window_);
	}

}
