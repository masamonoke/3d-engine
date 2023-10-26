#include "app.hpp"

namespace engine {
	void App::run() {
		while (!window_.shouldClose()) {
			glfwPollEvents();
		}
	}
}
