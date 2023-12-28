#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "engine_device.hpp"
#include "scene_object.hpp"
#include "pipeline.hpp"
#include "camera.hpp"
#include "frame_info.hpp"

#include <memory>
#include <vector>

namespace engine {

	class RenderSystem {
		public:
			RenderSystem(EngineDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
			~RenderSystem();

			RenderSystem(const RenderSystem&) = delete;
			RenderSystem &operator=(const RenderSystem&) = delete;
			RenderSystem(const RenderSystem&&) = delete;
			RenderSystem &&operator=(const RenderSystem&&) = delete;

			void renderSceneObjects(FrameInfo& frame_info, std::vector<SceneObject>& scene_objects);

		private:
			EngineDevice& device_;
			std::unique_ptr<Pipeline> pipeline_;
			VkPipelineLayout pipelineLayout_;

			void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
			void createPipeline(VkRenderPass render_pass);
	};

}

#endif // RENDER_SYSTEM_HPP
