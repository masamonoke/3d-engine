#include "app.hpp"

#include <stdexcept>

namespace engine {

	App::App() {
		loadModels();
		createPipelineLayout();
		createPipeline();
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
	}

	void App::createPipelineLayout() {
		VkPipelineLayoutCreateInfo layout_create_info {};
		layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_create_info.setLayoutCount = 0;
		layout_create_info.pSetLayouts = nullptr;
		layout_create_info.pushConstantRangeCount = 0;
		layout_create_info.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(device_.device(), &layout_create_info, nullptr, &pipelineLayout_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void App::createPipeline() {
		auto pipeline_config = Pipeline::defaultPipelineConfigInfo(swapChain_.width(), swapChain_.height());
		pipeline_config.renderPass = swapChain_.getRenderPass();
		pipeline_config.pipelineLayout = pipelineLayout_;
		pipeline_ = std::make_unique<Pipeline>(device_, "../shader/build/simple_shader.vert.spv", "../shader/build/simple_shader.frag.spv", pipeline_config);
	}

	void App::createCommandBuffers() {
		commandBuffers_.resize(swapChain_.imageCount());
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = device_.commandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

		if (vkAllocateCommandBuffers(device_.device(), &alloc_info, commandBuffers_.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}

		for (size_t i = 0; i < commandBuffers_.size(); i++) {
			VkCommandBufferBeginInfo begin_info {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(commandBuffers_[i], &begin_info) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer");
			}
			VkRenderPassBeginInfo render_pass_info {};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_info.renderPass = swapChain_.getRenderPass();
			render_pass_info.framebuffer = swapChain_.getFrameBuffer(i);
			render_pass_info.renderArea.offset = { 0, 0 };
			render_pass_info.renderArea.extent = swapChain_.getSwapChainExtent();
			std::array<VkClearValue, 2> clear_values {};
			clear_values[0].color = {{ 0.1f, 0.1f, 0.1f, 1.0f }};
			clear_values[1].depthStencil = { 1.0f, 0 };
			render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
			render_pass_info.pClearValues = clear_values.data();
			vkCmdBeginRenderPass(commandBuffers_[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
			pipeline_->bind(commandBuffers_[i]);
			model_->bind(commandBuffers_[i]);
			model_->draw(commandBuffers_[i]);
			vkCmdEndRenderPass(commandBuffers_[i]);
			if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS) {
				std::runtime_error("failed to record command buffer");
			}
		}
	}

	void App::drawFrame() {
		uint32_t image_idx;
		auto res = swapChain_.acquireNextImage(&image_idx);
		if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}
		res = swapChain_.submitCommandBuffers(&commandBuffers_[image_idx], &image_idx);
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

	void App::loadModels() {
		std::vector<Model::Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		/* std::vector<Model::Vertex> vertices = {}; */
		/* sierpinsky(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f}); */
		model_ = std::make_unique<Model>(device_, vertices);
	}
}
