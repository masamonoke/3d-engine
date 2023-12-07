#include "renderer.hpp"

#include <array>
#include <cassert>
#include <stdexcept>

namespace engine {
	Renderer::Renderer(Window& window, EngineDevice& device) : window_ { window }, device_ { device } {
		recreateSwapChain();
		createCmdBuffers();
	}

	Renderer::~Renderer() {
		freeCmdBuffers();
	}

	void Renderer::recreateSwapChain() {
		auto extent = window_.extent();
		while (extent.width == 0 || extent.height == 0) {
			extent = window_.extent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device_.device());
		if (swapChain_ == nullptr) {
			swapChain_ = std::make_unique<SwapChain>(device_, extent);
		} else {
			std::shared_ptr<SwapChain> old_swap_chain = std::move(swapChain_);
			swapChain_ = std::make_unique<SwapChain>(device_, extent, old_swap_chain);
			if (!old_swap_chain->compareSwapFormats(*swapChain_.get())) {
				throw std::runtime_error("Swap chain image (or depth) format has changed");
			}
		}
	}

	void Renderer::createCmdBuffers() {
		cmdBuffers_.resize(SwapChain::MAX_FRAMES);
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = device_.commandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(cmdBuffers_.size());

		if (vkAllocateCommandBuffers(device_.device(), &alloc_info, cmdBuffers_.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void Renderer::freeCmdBuffers() {
		vkFreeCommandBuffers(device_.device(), device_.commandPool(), static_cast<uint32_t>(cmdBuffers_.size()), cmdBuffers_.data());
		cmdBuffers_.clear();
	}

	VkCommandBuffer Renderer::beginFrame() {
		assert(!isFrameStarted_ && "Can't call beginFrame while already in progress");
		auto res = swapChain_->acquireNextImage(&curImageIdx_);
		if (res == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}
		if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}
		isFrameStarted_ = true;
		auto cmd_buf = currentCmdbuffer();
		VkCommandBufferBeginInfo begin_info {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(cmd_buf, &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}
		return cmd_buf;
	}

	void Renderer::endFrame() {
		assert(isFrameStarted_ && "Can't call endFrame while frame is not in progress");
		auto cmd_buf = currentCmdbuffer();
		if (vkEndCommandBuffer(cmd_buf) != VK_SUCCESS) {
			std::runtime_error("failed to record command buffer");
		}
		auto res = swapChain_->submitCommandBuffers(&cmd_buf, &curImageIdx_);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || window_.wasResized()) {
			window_.resetWindowResize();
			recreateSwapChain();
		} else if (res != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}
		isFrameStarted_ = false;
		curFrameIdx_ = (curFrameIdx_ + 1) % SwapChain::MAX_FRAMES;
	}

	void Renderer::beginSwapChainRenderPass(VkCommandBuffer cmd_buf) {
		assert(isFrameStarted_ && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(cmd_buf == currentCmdbuffer() && "Can't begin render pass on command buffer from a different frame");
		VkRenderPassBeginInfo render_pass_info {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = swapChain_->getRenderPass();
		render_pass_info.framebuffer = swapChain_->getFrameBuffer(curImageIdx_);
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = swapChain_->getSwapChainExtent();
		std::array<VkClearValue, 2> clear_values {};
		clear_values[0].color = {{ 0.01f, 0.01f, 0.01f, 1.0f }};
		clear_values[1].depthStencil = { 1.0f, 0 };
		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(cmd_buf, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain_->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain_->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, swapChain_->getSwapChainExtent()};
		vkCmdSetViewport(cmd_buf, 0, 1, &viewport);
		vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

	}

	void Renderer::endSwapChainRenderPass(VkCommandBuffer cmd_buf) {
		assert(isFrameStarted_ && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(cmd_buf == currentCmdbuffer() && "Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(cmd_buf);
	}
}
