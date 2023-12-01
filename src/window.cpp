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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window_ = glfwCreateWindow(
				width_,
				height_,
				title_.c_str(),
				nullptr,
				nullptr);
		glfwSetWindowUserPointer(window_, this);
		glfwSetFramebufferSizeCallback(window_, frameBufferResizedCallback);
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(window_);
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void Window::frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		w->frameBufferResized_ = true;
		w->width_ = width;
		w->height_ = height;
	}

}
