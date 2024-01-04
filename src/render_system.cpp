#include "render_system.hpp"

#include <glm/gtc/constants.hpp>

namespace engine {

	struct PushConstantData {
		glm::mat4 modelMatrix { 1.0F };
		glm::mat4 normalMatrix { 1.0F };
	};

	RenderSystem::RenderSystem(EngineDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout) : device_{ device }  { // NOLINT
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

	RenderSystem::~RenderSystem() {
		vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
	}


	void RenderSystem::renderSceneObjects(FrameInfo& frame_info) {
		pipeline_->bind(frame_info.cmdBuf);

		vkCmdBindDescriptorSets(frame_info.cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &frame_info.globalDescriptorSet, 0, nullptr);

		for (auto& kv : frame_info.sceneObjects) { // NOLINT
			auto& obj = kv.second;
			if (obj.model == nullptr) {
				continue;
			}
			PushConstantData push {};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();
			vkCmdPushConstants(
				frame_info.cmdBuf,
				pipelineLayout_,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PushConstantData),
				&push
			);
			obj.model->bind(frame_info.cmdBuf);
			obj.model->draw(frame_info.cmdBuf);
		}
	}

	void RenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
		VkPushConstantRange pushConstantRange {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstantData);

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

	void RenderSystem::createPipeline(VkRenderPass render_pass) {
		assert(pipelineLayout_ && "Cannot create pipeline before pipeline layout"); // NOLINT

		PipelineConfigInfo pipeline_config = Pipeline::defaultPipelineConfigInfo();
		pipeline_config.renderPass = render_pass;
		pipeline_config.pipelineLayout = pipelineLayout_;
		pipeline_ = std::make_unique<Pipeline>(device_, "../shader/build/simple_shader.vert.spv", "../shader/build/simple_shader.frag.spv", pipeline_config);

	}

}
