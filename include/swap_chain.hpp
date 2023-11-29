#ifndef SWAP_CHAIN_HPP
#define SWAP_CHAIN_HPP

#include "engine_device.hpp"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace engine {
	class SwapChain {
		public:
			static constexpr int MAX_FRAMES = 2;

			SwapChain(EngineDevice& engine_device, VkExtent2D window_extent);
			~SwapChain();
			SwapChain(const SwapChain&) = delete;
			SwapChain& operator=(const SwapChain&) = delete;

			VkFramebuffer getFrameBuffer(int index) {
				return swapChainFrameBuffers_[index];
			}
			VkRenderPass getRenderPass() {
				return renderPass_;
			}
			VkImageView getImageView(int index) {
				return swapChainImageViews_[index];
			}
			size_t imageCount() {
				return swapChainImages_.size();
			}
			VkFormat getSwapChainImageFormat() {
				return swapChainImageFormat_;
			}
			VkExtent2D getSwapChainExtent() {
				return swapChainExtent_;
			}
			uint32_t width() {
				return swapChainExtent_.width;
			}
			uint32_t height() {
				return swapChainExtent_.height;
			}

			float extentAspectRatio() {
				return static_cast<float>(swapChainExtent_.width) / static_cast<float>(swapChainExtent_.height);
			}
			VkFormat findDepthFormat();
			VkResult acquireNextImage(uint32_t* image_idx);
			VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* image_idx);


		private:
			VkFormat swapChainImageFormat_;
			VkExtent2D swapChainExtent_;
			std::vector<VkFramebuffer> swapChainFrameBuffers_;
			VkRenderPass renderPass_;
			std::vector<VkImage> depthImages_;
			std::vector<VkDeviceMemory> depthImageMemories_;
			std::vector<VkImageView> depthImageViews_;
			std::vector<VkImage> swapChainImages_;
			std::vector<VkImageView> swapChainImageViews_;
			EngineDevice& device_;
			VkExtent2D windowExtent_;
			VkSwapchainKHR swapChain_;
			std::vector<VkSemaphore> imageAvailableSemaphores_;
			std::vector<VkSemaphore> renderFinishedSemaphores_;
			std::vector<VkFence> inFlightFences_;
			std::vector<VkFence> imagesInFlight_;
			size_t currentFrame_ = 0;

			void createSwapChain();
			void createImageViews();
			void createDepthResources();
			void createRenderPass();
			void createFrameBuffers();
			void createSyncObjects();

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& capabilities);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	};
}

#endif // SWAP_CHAIN_HPP
