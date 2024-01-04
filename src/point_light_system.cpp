#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

namespace engine {

	struct PointLightPushConstants {
		glm::vec4 position {};
		glm::vec4 color {};
		float radius;
	};

	PointLightSystem::PointLightSystem(EngineDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout) : device_{ device }  { // NOLINT
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

	PointLightSystem::~PointLightSystem() {
		vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
	}


	void PointLightSystem::render(FrameInfo& frame_info) {
		pipeline_->bind(frame_info.cmdBuf);

		vkCmdBindDescriptorSets(frame_info.cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &frame_info.globalDescriptorSet, 0, nullptr);

		const uint32_t vertices_count = 6;
		for (auto& kv : frame_info.sceneObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) {
				continue;
			}
			PointLightPushConstants push {};
			push.position = glm::vec4(obj.transform.translation, 1.0F);
			push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			push.radius = obj.transform.scale.x;
			vkCmdPushConstants(frame_info.cmdBuf, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);

			vkCmdDraw(frame_info.cmdBuf, vertices_count, 1, 0, 0);
		}
	}

	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
		VkPushConstantRange pushConstantRange {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptor_set_layouts { global_set_layout };

		VkPipelineLayoutCreateInfo layout_create_info {};
		layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		layout_create_info.pSetLayouts = descriptor_set_layouts.data();
		layout_create_info.pushConstantRangeCount = 1;
		layout_create_info.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(device_.device(), &layout_create_info, nullptr, &pipelineLayout_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void PointLightSystem::createPipeline(VkRenderPass render_pass) {
		assert(pipelineLayout_ && "Cannot create pipeline before pipeline layout"); // NOLINT

		PipelineConfigInfo pipeline_config = Pipeline::defaultPipelineConfigInfo();
		pipeline_config.attributeDescriptions.clear();
		pipeline_config.bindingDescriptions.clear();
		pipeline_config.renderPass = render_pass;
		pipeline_config.pipelineLayout = pipelineLayout_;
		pipeline_ = std::make_unique<Pipeline>(device_, "../shader/build/point_light.vert.spv", "../shader/build/point_light.frag.spv", pipeline_config);

	}

	void PointLightSystem::update(FrameInfo& frame_info, GlobalUbo& ubo) {
		int light_idx = 0;
		auto rotation = glm::rotate(glm::mat4(1.0F), frame_info.frameTime, { 0.0F, -1.0F, 0.0F });
		for (auto& kv : frame_info.sceneObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) {
				continue;
			}
			assert(light_idx < MAX_LIGHTS && "Point lights exceeded max limit"); // NOLINT
			obj.transform.translation = glm::vec3(rotation * glm::vec4(obj.transform.translation, 1.0F));
			ubo.pointLights[light_idx].position = glm::vec4(obj.transform.translation, 1.0F);
			ubo.pointLights[light_idx].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			light_idx++;
		}
		ubo.lightsNum = light_idx;
	}

} // namespace engine
