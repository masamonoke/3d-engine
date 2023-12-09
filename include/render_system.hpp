#ifndef RENDER_SYSTEM_HPP
#define RENDER_SYSTEM_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "engine_device.hpp"
#include "scene_object.hpp"
#include "pipeline.hpp"
#include "camera.hpp"

#include <memory>
#include <vector>

namespace engine {

	class RenderSystem {
		public:
			RenderSystem(EngineDevice& device, VkRenderPass render_pass);
			~RenderSystem();

			RenderSystem(const RenderSystem&) = delete;
			RenderSystem &operator=(const RenderSystem&) = delete;

			void renderSceneObjects(VkCommandBuffer cmd_buf, std::vector<SceneObject>& scene_objects, const Camera& camera);

		private:
			EngineDevice& device_;
			std::unique_ptr<Pipeline> pipeline_;
			VkPipelineLayout pipelineLayout_;

			void createPipelineLayout();
			void createPipeline(VkRenderPass render_pass);
	};

}

#endif // RENDER_SYSTEM_HPP
