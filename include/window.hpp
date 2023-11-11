#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace engine {
	class Window {
		public:
			Window(int w, int h, std::string title);
			~Window();
			Window(const Window &) = delete;
			Window &operator=(const Window &) = delete;

			bool shouldClose();
			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
			VkExtent2D extent() {
				return { static_cast<uint32_t>(width_), static_cast<uint32_t>(height_) };
			}

		private:
			void initWindow();

			const int width_;
			const int height_;
			GLFWwindow* window_;
			std::string title_;
	};
}

#endif // WINDOW_H
