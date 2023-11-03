#ifndef APP_H
#define APP_H

#include <vulkan/vulkan.h>

#include "window.hpp"
#include "pipeline.hpp"
#include "engine_device.hpp"

namespace engine {
	class App {
		public:
			static constexpr int WIDTH = 1200;
			static constexpr int HEIGHT = 700;

			void run();

		private:
			Window window_ { WIDTH, HEIGHT, "App" };
			EngineDevice device_ { window_ };
			Pipeline pipeline_ {
				device_,
				"../shader/build/simple_shader.vert.spv",
				"../shader/build/simple_shader.frag.spv",
				Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT) };
	};
}

#endif // APP_H
