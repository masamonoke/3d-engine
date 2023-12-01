#include "swap_chain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace engine {
	SwapChain::SwapChain(EngineDevice& device, VkExtent2D extent) : device_{ device }, windowExtent_{ extent } {
		init();
	}

	SwapChain::SwapChain(EngineDevice& device, VkExtent2D extent, std::shared_ptr<SwapChain> previous) : device_{ device }, windowExtent_{ extent },
		oldSwapChain_{ previous } {
		init();
		oldSwapChain_ = nullptr;
	}

	SwapChain::~SwapChain() {
		for (auto image_view : swapChainImageViews_) {
			vkDestroyImageView(device_.device(), image_view, nullptr);
		}
		swapChainImageViews_.clear();
		if (swapChain_ != nullptr) {
			vkDestroySwapchainKHR(device_.device(), swapChain_, nullptr);
			swapChain_ = nullptr;
		}
		for (size_t i = 0; i < depthImages_.size(); i++) {
			vkDestroyImageView(device_.device(), depthImageViews_[i], nullptr);
			vkDestroyImage(device_.device(), depthImages_[i], nullptr);
			vkFreeMemory(device_.device(), depthImageMemories_[i], nullptr);
		}
		for (auto frame_buffer : swapChainFrameBuffers_) {
			vkDestroyFramebuffer(device_.device(), frame_buffer, nullptr);
		}
		vkDestroyRenderPass(device_.device(), renderPass_, nullptr);
		for (size_t i = 0; i < MAX_FRAMES; i++) {
			vkDestroySemaphore(device_.device(), renderFinishedSemaphores_[i], nullptr);
			vkDestroySemaphore(device_.device(), imageAvailableSemaphores_[i], nullptr);
			vkDestroyFence(device_.device(), inFlightFences_[i], nullptr);
		}
	}

	VkResult SwapChain::acquireNextImage(uint32_t *image_idx) {
		vkWaitForFences(device_.device(), 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
		auto res = vkAcquireNextImageKHR(
			device_.device(),
			swapChain_,
			UINT64_MAX,
			imageAvailableSemaphores_[currentFrame_],
			VK_NULL_HANDLE,
			image_idx);
		return res;
	}

	VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *image_idx) {
		if (imagesInFlight_[*image_idx] != VK_NULL_HANDLE) {
			vkWaitForFences(device_.device(), 1, &imagesInFlight_[*image_idx], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight_[*image_idx] = inFlightFences_[currentFrame_];
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkSemaphore wait_semaphores[] = { imageAvailableSemaphores_[currentFrame_] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = buffers;

		VkSemaphore signal_semaphores[] = { renderFinishedSemaphores_[currentFrame_] };
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		vkResetFences(device_.device(), 1, &inFlightFences_[currentFrame_]);
		if (vkQueueSubmit(device_.graphicsQueue(), 1, &submit_info, inFlightFences_[currentFrame_]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer");
		}

		VkPresentInfoKHR present_info {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swap_chains[] = { swapChain_ };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swap_chains;
		present_info.pImageIndices = image_idx;

		auto res = vkQueuePresentKHR(device_.presentQueue(), &present_info);
		currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES;
		return res;
	}

	void SwapChain::createSwapChain() {
		SwapChainSupportDetails swap_chain_support = device_.swapChainSupport();
		VkSurfaceFormatKHR surface_format = this->chooseSwapSurfaceFormat(swap_chain_support.formats);
		VkPresentModeKHR present_mode = this->chooseSwapPresentMode(swap_chain_support.presentModes);
		VkExtent2D extent = this->chooseSwapExtent(swap_chain_support.capabilites);

		uint32_t image_count = swap_chain_support.capabilites.minImageCount + 1;
		if (swap_chain_support.capabilites.maxImageCount > 0 && image_count > swap_chain_support.capabilites.maxImageCount) {
			image_count = swap_chain_support.capabilites.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = device_.surface();
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto indices = device_.findPhysicalQueueFamilies();
		uint32_t queue_family_indices[] = { indices.graphicsFamily, indices.presentFamily };
		if (indices.graphicsFamily != indices.presentFamily) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		} else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}

		create_info.preTransform = swap_chain_support.capabilites.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = oldSwapChain_ == nullptr ? VK_NULL_HANDLE : oldSwapChain_->swapChain_;
		if (vkCreateSwapchainKHR(device_.device(), &create_info, nullptr, &swapChain_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain");
		}

		vkGetSwapchainImagesKHR(device_.device(), swapChain_, &image_count, nullptr);
		swapChainImages_.resize(image_count);
		vkGetSwapchainImagesKHR(device_.device(), swapChain_, &image_count, swapChainImages_.data());
		swapChainImageFormat_ = surface_format.format;
		swapChainExtent_ = extent;
	}

	void SwapChain::createImageViews() {
		swapChainImageViews_.resize(swapChainImages_.size());
		for (size_t i = 0; i < swapChainImages_.size(); i++) {
			VkImageViewCreateInfo view_info {};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = swapChainImages_[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = swapChainImageFormat_;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device_.device(), &view_info, nullptr, &swapChainImageViews_[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view");
			}
		}
	}

	void SwapChain::createRenderPass() {
		VkAttachmentDescription depth_attachment{};
		depth_attachment.format = this->findDepthFormat();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref{};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription color_attachment = {};
		color_attachment.format = this->getSwapChainImageFormat();
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask =
		  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask =
		  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask =
		  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;

		if (vkCreateRenderPass(device_.device(), &render_pass_info, nullptr, &renderPass_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void SwapChain::createFrameBuffers() {
		swapChainFrameBuffers_.resize(this->imageCount());
		for (size_t i = 0; i < this->imageCount(); i++) {
			std::array<VkImageView, 2> attachments = { swapChainImageViews_[i], depthImageViews_[i] };
			auto swap_chain_extent = this->getSwapChainExtent();
			VkFramebufferCreateInfo frame_buffer_info {};
			frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_info.renderPass = renderPass_;
			frame_buffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			frame_buffer_info.pAttachments = attachments.data();
			frame_buffer_info.width = swap_chain_extent.width;
			frame_buffer_info.height = swap_chain_extent.height;
			frame_buffer_info.layers = 1;

			if (vkCreateFramebuffer(device_.device(), &frame_buffer_info, nullptr, &swapChainFrameBuffers_[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create frame buffer");
			}
		}
	}

	void SwapChain::createDepthResources() {
		auto depth_format = this->findDepthFormat();
		auto swap_chain_extent = this->getSwapChainExtent();
		depthImages_.resize(this->imageCount());
		depthImageMemories_.resize(this->imageCount());
		depthImageViews_.resize(this->imageCount());
		for (size_t i = 0; i < depthImages_.size(); i++) {
			VkImageCreateInfo image_info {};
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.extent.width = swap_chain_extent.width;
			image_info.extent.height = swap_chain_extent.height;
			image_info.extent.depth = 1;
			image_info.mipLevels = 1;
			image_info.arrayLayers = 1;
			image_info.format = depth_format;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_info.flags = 0;

			device_.createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages_[i], depthImageMemories_[i]);
			VkImageViewCreateInfo view_info {};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = depthImages_[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = depth_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device_.device(), &view_info, nullptr, &depthImageViews_[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view");
			}
		}
	}

	void SwapChain::createSyncObjects() {
		imageAvailableSemaphores_.resize(MAX_FRAMES);
		renderFinishedSemaphores_.resize(MAX_FRAMES);
		inFlightFences_.resize(MAX_FRAMES);
		imagesInFlight_.resize(this->imageCount(), VK_NULL_HANDLE);
		VkSemaphoreCreateInfo semaphore_info {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fence_info {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < MAX_FRAMES; i++) {
			if (vkCreateSemaphore(device_.device(), &semaphore_info, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device_.device(), &semaphore_info, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
				vkCreateFence(device_.device(), &fence_info, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame");
			}
		}
	}

	VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
		for (const auto& format : available_formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}
		return available_formats[0];
	}

	VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
		for (const auto& mode: available_present_modes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "present box: mailbox" << "\n";
				return mode;
			}
		}
		std::cout << "present mode: v-sync" << "\n";
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		auto actual_extent = windowExtent_;
		actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));
		return actual_extent;
	}

	VkFormat SwapChain::findDepthFormat() {
		return device_.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	void SwapChain::init() {
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDepthResources();
		createFrameBuffers();
		createSyncObjects();
	}

}
