#ifndef APP_H
#define APP_H

#include <memory>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "window.hpp"
#include "pipeline.hpp"
#include "engine_device.hpp"
#include "swap_chain.hpp"
/* #include "model.hpp" */
#include "scene_object.hpp"

namespace engine {

	struct PushConstantData {
		glm::mat2 transform { 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

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
			PipelineConfigInfo configInfo {};
			std::unique_ptr<SwapChain> swapChain_;
			std::unique_ptr<Pipeline> pipeline_;
			VkPipelineLayout pipelineLayout_;
			std::vector<VkCommandBuffer> commandBuffers_;
			/* std::unique_ptr<Model> model_; */
			std::vector<SceneObject> sceneObjects_;

			void createPipelineLayout();
			void createPipeline();
			void createCommandBuffers();
			void drawFrame();
			void loadSceneObjects();
			void recreateSwapChain();
			void recordCommandBuffer(int image_index);
			void freeCommandBuffers();
			void renderSceneObjects(VkCommandBuffer cmd_buf);
	};
}

#endif // APP_H
