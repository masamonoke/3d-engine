#ifndef APP_H
#define APP_H

#include <memory>

#include <vulkan/vulkan.h>

#include "window.hpp"
#include "pipeline.hpp"
#include "engine_device.hpp"
#include "swap_chain.hpp"

namespace engine {
	class App {
		public:
			static constexpr int WIDTH = 1200;
			static constexpr int HEIGHT = 700;

			App();
			~App();
			App(const App&) = delete;
			App &operator=(const App&) = delete;

			void run();

		private:
			Window window_ { WIDTH, HEIGHT, "App" };
			EngineDevice device_ { window_ };
			PipelineConfigInfo configInfo {};
			SwapChain swapChain_ { device_, window_.extent() };
			std::unique_ptr<Pipeline> pipeline_;
			VkPipelineLayout pipelineLayout_;
			std::vector<VkCommandBuffer> commandBuffers_;

			void createPipelineLayout();
			void createPipeline();
			void createCommandBuffers();
			void drawFrame();
	};
}

#endif // APP_H
