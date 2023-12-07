#ifndef APP_H
#define APP_H

#include <memory>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "window.hpp"
#include "engine_device.hpp"
#include "scene_object.hpp"
#include "renderer.hpp"

namespace engine {

	class App {
		public:
			static constexpr int WIDTH = 1280;
			static constexpr int HEIGHT = 720;

			App();
			~App();
			App(const App&) = delete;
			App& operator=(const App&) = delete;

			void run();

		private:
			Window window_ { WIDTH, HEIGHT, "App" };
			EngineDevice device_ { window_ };
			std::vector<SceneObject> sceneObjects_;
			Renderer renderer_ { window_, device_ };

			void loadSceneObjects();
	};

}

#endif // APP_H
