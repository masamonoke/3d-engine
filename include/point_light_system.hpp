#ifndef POINT_LIGHT_SYSTEM_HPP
#define POINT_LIGHT_SYSTEM_HPP

#include "camera.hpp"
#include "engine_device.hpp"
#include "frame_info.hpp"
#include "scene_object.hpp"
#include "pipeline.hpp"

#include <memory>
#include <vector>

namespace engine {

	class PointLightSystem {
		public:
			PointLightSystem(EngineDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
			~PointLightSystem();

			PointLightSystem(const PointLightSystem&) = delete;
			PointLightSystem &operator=(const PointLightSystem&) = delete;
			PointLightSystem(const PointLightSystem&&) = delete;
			PointLightSystem &&operator=(const PointLightSystem&&) = delete;

			void render(FrameInfo& frame_info);
			void update(FrameInfo& frame_info, GlobalUbo& ubo);

		private:
			void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
			void createPipeline(VkRenderPass render_pass);

			EngineDevice& device_;
			std::unique_ptr<Pipeline> pipeline_;
			VkPipelineLayout pipelineLayout_;
	};

} // namespace engine

#endif // POINT_LIGHT_SYSTEM_HPP
