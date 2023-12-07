#include "render_system.hpp"

#include <glm/gtc/constants.hpp>

namespace engine {

	struct PushConstantData {
		glm::mat2 transform { 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	RenderSystem::RenderSystem(EngineDevice& device, VkRenderPass render_pass) : device_{ device }  {
		createPipelineLayout();
		createPipeline(render_pass);
	}

	RenderSystem::~RenderSystem() {
		vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
	}


	void RenderSystem::renderSceneObjects(VkCommandBuffer cmd_buf, std::vector<SceneObject>& scene_objects) {
		pipeline_->bind(cmd_buf);
		for (auto& obj : scene_objects) {
			obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());
			PushConstantData push {};
			push.offset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();
			vkCmdPushConstants(
				cmd_buf,
				pipelineLayout_,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PushConstantData),
				&push
			);
			obj.model->bind(cmd_buf);
			obj.model->draw(cmd_buf);
		}
	}

	void RenderSystem::createPipelineLayout() {
		VkPushConstantRange pushConstantRange {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstantData);

		VkPipelineLayoutCreateInfo layout_create_info {};
		layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_create_info.setLayoutCount = 0;
		layout_create_info.pSetLayouts = nullptr;
		layout_create_info.pushConstantRangeCount = 1;
		layout_create_info.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(device_.device(), &layout_create_info, nullptr, &pipelineLayout_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void RenderSystem::createPipeline(VkRenderPass render_pass) {
		assert(pipelineLayout_ != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipeline_config = Pipeline::defaultPipelineConfigInfo();
		pipeline_config.renderPass = render_pass;
		pipeline_config.pipelineLayout = pipelineLayout_;
		pipeline_ = std::make_unique<Pipeline>(device_, "../shader/build/simple_shader.vert.spv", "../shader/build/simple_shader.frag.spv", pipeline_config);

	}

}
