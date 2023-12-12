#include "render_system.hpp"

#include <glm/gtc/constants.hpp>

namespace engine {

	struct PushConstantData {
		glm::mat4 transform { 1.0F };
		glm::mat4 normalMatrix { 1.0F };
	};

	RenderSystem::RenderSystem(EngineDevice& device, VkRenderPass render_pass) : device_{ device }  {
		createPipelineLayout();
		createPipeline(render_pass);
	}

	RenderSystem::~RenderSystem() {
		vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
	}


	void RenderSystem::renderSceneObjects(VkCommandBuffer cmd_buf, std::vector<SceneObject>& scene_objects, const Camera& camera) {
		pipeline_->bind(cmd_buf);

		auto projection_view = camera.projection() * camera.view();

		for (auto& obj : scene_objects) {
			PushConstantData push {};
			auto model_matrix = obj.transform.mat4();
			push.transform = projection_view * model_matrix;
			push.normalMatrix = obj.transform.normalMatrix();
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
