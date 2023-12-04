#include "app.hpp"

#include <stdexcept>
#include <iostream>

#include <glm/gtc/constants.hpp>

namespace engine {

	App::App() {
		loadSceneObjects();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	App::~App() {
		vkDestroyPipelineLayout(device_.device(), pipelineLayout_, nullptr);
	}

	void App::run() {
		while (!window_.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(device_.device());
	}

	void App::createPipelineLayout() {
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

	void App::createPipeline() {
		assert(swapChain_ != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout_ != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipeline_config = Pipeline::defaultPipelineConfigInfo();
		pipeline_config.renderPass = swapChain_->getRenderPass();
		pipeline_config.pipelineLayout = pipelineLayout_;
		pipeline_ = std::make_unique<Pipeline>(device_, "../shader/build/simple_shader.vert.spv", "../shader/build/simple_shader.frag.spv", pipeline_config);
	}

	void App::createCommandBuffers() {
		commandBuffers_.resize(swapChain_->imageCount());
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = device_.commandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

		if (vkAllocateCommandBuffers(device_.device(), &alloc_info, commandBuffers_.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void App::recreateSwapChain() {
		auto extent = window_.extent();
		while (extent.width == 0 || extent.height == 0) {
			extent = window_.extent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device_.device());
		if (swapChain_ == nullptr) {
			swapChain_ = std::make_unique<SwapChain>(device_, extent);
		} else {
			swapChain_ = std::make_unique<SwapChain>(device_, extent, std::move(swapChain_));
			if (swapChain_->imageCount() != commandBuffers_.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
		createPipeline();
	}

	void App::recordCommandBuffer(int image_index) {
		VkCommandBufferBeginInfo begin_info {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffers_[image_index], &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}
		VkRenderPassBeginInfo render_pass_info {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = swapChain_->getRenderPass();
		render_pass_info.framebuffer = swapChain_->getFrameBuffer(image_index);
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = swapChain_->getSwapChainExtent();
		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = {{ 0.01f, 0.01f, 0.01f, 1.0f }};
		clear_values[1].depthStencil = { 1.0f, 0 };
		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(commandBuffers_[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain_->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain_->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, swapChain_->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffers_[image_index], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers_[image_index], 0, 1, &scissor);

		renderSceneObjects(commandBuffers_[image_index]);

		vkCmdEndRenderPass(commandBuffers_[image_index]);
		if (vkEndCommandBuffer(commandBuffers_[image_index]) != VK_SUCCESS) {
			std::runtime_error("failed to record command buffer");
		}
	}

	void App::drawFrame() {
		uint32_t image_idx;
		auto res = swapChain_->acquireNextImage(&image_idx);
		if (res == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}
		recordCommandBuffer(image_idx);
		res = swapChain_->submitCommandBuffers(&commandBuffers_[image_idx], &image_idx);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || window_.wasResized()) {
			window_.resetWindowResize();
			recreateSwapChain();
			return;
		}
		if (res != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}
	}

	void sierpinsky(std::vector<Model::Vertex>& vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top) {
		if (depth <= 0) {
			vertices.push_back({{top}, {1.0f, 0.0f, 0.0f}});
			vertices.push_back({{right}, {0.0f, 1.0f, 0.0f}});
			vertices.push_back({{left}, {0.0f, 0.0f, 1.0f}});
		} else {
			auto left_top = 0.5f * (left + top);
			auto right_top = 0.5f * (right + top);
			auto left_right = 0.5f * (left + right);
			sierpinsky(vertices, depth - 1, left, left_right, left_top);
			sierpinsky(vertices, depth - 1, left_right, right, right_top);
			sierpinsky(vertices, depth - 1, left_top, right_top, top);
		}
	}

	void App::loadSceneObjects() {
		std::vector<Model::Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		/* std::vector<Model::Vertex> vertices = {}; */
		/* sierpinsky(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f}); */
		auto model = std::make_shared<Model>(device_, vertices);
		auto triangle = SceneObject::createObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2d.translation.x = 0.2f;
		triangle.transform2d.scale = { 2.0f, 0.5f };
		triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();
		sceneObjects_.push_back(std::move(triangle));
	}

	void App::freeCommandBuffers() {
		vkFreeCommandBuffers(device_.device(), device_.commandPool(), static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
		commandBuffers_.clear();
	}

	void App::renderSceneObjects(VkCommandBuffer cmd_buf) {
		pipeline_->bind(cmd_buf);
		for (auto& obj : sceneObjects_) {
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

}
