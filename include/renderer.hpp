#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "engine_device.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace engine {

	class Renderer {
		public:
			Renderer(Window& window, EngineDevice& device);
			~Renderer();

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			VkCommandBuffer beginFrame();
			void endFrame();
			void beginSwapChainRenderPass(VkCommandBuffer cmd_buf);
			void endSwapChainRenderPass(VkCommandBuffer cmd_buf);

			bool isFrameInProgress() { return isFrameStarted_; }

			VkCommandBuffer currentCmdbuffer() const {
				assert(isFrameStarted_ && "Cannot get command buffer when frame not in progress");
				return cmdBuffers_[curFrameIdx_];
			}

			VkRenderPass swapChainRenderPass() const {
				return swapChain_->getRenderPass();
			}

			int frameIdx() const {
				assert(isFrameStarted_ && "Cannot get frame index when frame not in progress");
				return curFrameIdx_;
			}

			float aspectRatio() const { return swapChain_->extentAspectRatio(); }

		private:
			Window& window_;
			EngineDevice& device_;
			std::unique_ptr<SwapChain> swapChain_;
			std::vector<VkCommandBuffer> cmdBuffers_;
			uint32_t curImageIdx_;
			bool isFrameStarted_;
			int curFrameIdx_;

			void createCmdBuffers();
			void freeCmdBuffers();
			void recreateSwapChain();
	};

}

#endif // RENDERER_HPP
