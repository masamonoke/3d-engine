#ifndef ENGINE_DEVICE_HPP
#define ENGINE_DEVICE_HPP

#include <string>
#include <vector>

#include "window.hpp"
#include <vulkan/vulkan_beta.h>

namespace engine {

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilites;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		uint32_t graphicsFamily;
		uint32_t presentFamily;
		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;
		bool isComplete() {
			return graphicsFamilyHasValue && presentFamilyHasValue;
		}
	};

	class EngineDevice {
		public:
#ifdef NDEBUG
			const bool enabledValidationLayers = false;
#else
			const bool enabledValidationLayers = true;
#endif
			VkPhysicalDeviceProperties properties;

			EngineDevice(Window& window);
			~EngineDevice();

			EngineDevice(const EngineDevice&) = delete;
			EngineDevice& operator=(const EngineDevice&) = delete;
			EngineDevice(EngineDevice&&) = delete;
			EngineDevice& operator=(EngineDevice&&) = delete;

			VkCommandPool commandPool() { return commandPool_; }
			VkDevice device() { return device_; }
			VkSurfaceKHR surface() { return surface_; }
			VkQueue graphicsQueue() { return graphicsQueue_; }
			VkQueue presentQueue() { return presentQueue_; }
			SwapChainSupportDetails swapChainSupport();
			uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
			QueueFamilyIndices findPhysicalQueueFamilies();
			VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
			void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buf, VkDeviceMemory& buf_memory);
			VkCommandBuffer beginSingleTimeCommands();
			void endSingleTimeCommands(VkCommandBuffer command_buf);
			void copyBuffer(VkBuffer src_buf, VkBuffer dst_buf, VkDeviceSize size);
			void copyBufferToImage(VkBuffer buf, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);
			void createImageWithInfo(const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory);


		private:
			Window& window_;
			VkCommandPool commandPool_;
			VkDevice device_;
			VkSurfaceKHR surface_;
			VkQueue graphicsQueue_;
			VkQueue presentQueue_;
			VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
			const std::vector<const char*> validationLayers_ = { "VK_LAYER_KHRONOS_validation" };
			const std::vector<const char*> deviceExtensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME };
			VkInstance instance_;
			VkDebugUtilsMessengerEXT debugMessenger_;


			SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);
			void createInstance();
			void setupDebugMessenger();
			void createSurface();
			void pickPhysicalDevice();
			void createLogicalDevice();
			void createCommandPool();
			bool isDeviceSuitable(VkPhysicalDevice device);
			std::vector<const char*> getRequiredExtensions();
			bool checkValidationLayerSupport();
			QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
			void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info);
			void hasGLFWRequiredInstanceExtensions();
			bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	};
}

#endif // ENGINE_DEVICE_HPP
