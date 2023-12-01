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
			Window& operator=(const Window &) = delete;

			bool shouldClose();
			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
			VkExtent2D extent() {
				return { static_cast<uint32_t>(width_), static_cast<uint32_t>(height_) };
			}
			bool wasResized() { return frameBufferResized_; }
			void resetWindowResize() { frameBufferResized_ = false; }

		private:
			static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
			int width_;
			int height_;
			bool frameBufferResized_ = false;
			void initWindow();
			static int w_;
			static int h_;
			static int frame_buffer_resized_;

			GLFWwindow* window_;
			std::string title_;
	};
}

#endif // WINDOW_H
