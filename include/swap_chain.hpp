#ifndef SWAP_CHAIN_HPP
#define SWAP_CHAIN_HPP

#include "engine_device.hpp"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>

namespace engine {

	class SwapChain {
		public:
			static constexpr int MAX_FRAMES = 2;

			SwapChain(EngineDevice& engine_device, VkExtent2D window_extent);
			SwapChain(EngineDevice& engine_device, VkExtent2D window_extent, std::shared_ptr<SwapChain> previous);
			~SwapChain();
			SwapChain(const SwapChain&) = delete;
			SwapChain& operator=(const SwapChain&) = delete;

			VkFormat findDepthFormat();
			VkResult acquireNextImage(uint32_t* image_idx);
			VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* image_idx);

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

			bool compareSwapFormats(const SwapChain& other) const {
				return other.swapChainDepthFormat_ == this->swapChainDepthFormat_ && other.swapChainImageFormat_ == this->swapChainImageFormat_;
			}

		private:
			VkFormat swapChainImageFormat_;
			VkFormat swapChainDepthFormat_;
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
			std::shared_ptr<SwapChain> oldSwapChain_;

			void createSwapChain();
			void createImageViews();
			void createDepthResources();
			void createRenderPass();
			void createFrameBuffers();
			void createSyncObjects();

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& capabilities);
			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			void init();
	};

}

#endif // SWAP_CHAIN_HPP
