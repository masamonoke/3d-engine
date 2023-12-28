#include "engine_device.hpp"
#include "utils.hpp"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>


namespace engine {

	namespace {

		VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
			VkDebugUtilsMessageTypeFlagsEXT message_type,
			const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
			void* user_data) {
			UNUSED(message_type);
			UNUSED(user_data);
			switch (message_severity) {
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
					std::cerr << "[DIAGNOSTIC] ";
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					std::cerr << "[INFO] ";
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					std::cerr << "[ERROR] ";
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					std::cerr << "[WARNING] ";
				default:
					break;
			}
			std::cerr << "validation layer: " << callback_data->pMessage << std::endl;
			return VK_FALSE;
		}

		VkResult create_debug_utils_messenger_ext(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* create_info,
			const VkAllocationCallbacks* allocator,
			VkDebugUtilsMessengerEXT* debug_messenger) {
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr) {
				return func(instance, create_info, allocator, debug_messenger);
			} else {
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		void destroy_debug_utils_messenger_ext(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debug_messenger,
			const VkAllocationCallbacks* allocator) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(instance, debug_messenger, allocator);
			}
		}
	}

	EngineDevice::EngineDevice(Window& window): window_{window}  {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	EngineDevice::~EngineDevice() {
		vkDestroyCommandPool(device_, commandPool_, nullptr);
		vkDestroyDevice(device_, nullptr);
		if (enabledValidationLayers) {
			destroy_debug_utils_messenger_ext(instance_, debugMessenger_, nullptr);
		}
		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		vkDestroyInstance(instance_, nullptr);
	}

	void EngineDevice::createInstance() {
		if (enabledValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layer requested but not available");
		}

		VkApplicationInfo app_info {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "3D model viewer";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "No engine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;

		auto extensions = getRequiredExtensions();
		create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();


		VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
		if (enabledValidationLayers) {
			create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
			create_info.ppEnabledLayerNames = validationLayers_.data();
			populateDebugMessengerCreateInfo(debug_create_info);
			create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
		} else {
			create_info.enabledLayerCount = 0;
			create_info.pNext = nullptr;
		}

		if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance");
		}

		hasGLFWRequiredInstanceExtensions();
	}

	void EngineDevice::pickPhysicalDevice() {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
		if (device_count == 0)  {
			throw std::runtime_error("failed to find GPUs with Vulkan support");
		}
		std::cout << "Device count: " << device_count << std::endl;
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice_ = device;
				break;
			}
		}

		if (physicalDevice_ == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU");
		}

		vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
		std::cout << "physical device: " << properties.deviceName << std::endl;
	}

	void EngineDevice::createLogicalDevice() {
		auto indices = findQueueFamilies(physicalDevice_);
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		const std::set<uint32_t> unique_queue_families = { indices.graphicsFamily, indices.presentFamily };
		auto queue_priority = 1.F;
		for (const auto queue_family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}
		VkPhysicalDeviceFeatures device_features {};
		device_features.samplerAnisotropy = VK_TRUE;
		VkDeviceCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.pEnabledFeatures = &device_features;
		create_info.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions_.size());
		create_info.ppEnabledExtensionNames = deviceExtensions_.data();

		if (this->enabledValidationLayers) {
			create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
			create_info.ppEnabledLayerNames = validationLayers_.data();
		} else {
			create_info.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice_, &create_info, nullptr, &device_) != VK_SUCCESS) {
			throw std::runtime_error("failed to load logical device");
		}

		vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
	}

	void EngineDevice::createCommandPool() {
		auto queueFamilyIndices = findPhysicalQueueFamilies();
		VkCommandPoolCreateInfo pool_info {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(device_, &pool_info, nullptr, &commandPool_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool");
		}
	}

	void EngineDevice::createSurface() {
		window_.createWindowSurface(instance_, &surface_);
	}

	bool EngineDevice::isDeviceSuitable(VkPhysicalDevice device) {
		auto indices = findQueueFamilies(device);
		auto extension_supported = checkDeviceExtensionSupport(device);
		auto swap_chain_adequate = false;
		if (extension_supported) {
			auto swap_chain_support = querySwapChainSupport(device);
			swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.presentModes.empty();
		}
		VkPhysicalDeviceFeatures supported_features;
		vkGetPhysicalDeviceFeatures(device, &supported_features);
		return indices.isComplete() && extension_supported && swap_chain_adequate && supported_features.samplerAnisotropy != 0;
	}

	void EngineDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
		create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debug_callback;
		create_info.pUserData = nullptr;
	}

	bool EngineDevice::checkValidationLayerSupport() {
		uint32_t layer_count = {};
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
		for (const char* layer_name : validationLayers_) {
			auto layer_found = false;
			for (const auto& layer_properties : available_layers) {
				if (strcmp(layer_name, layer_properties.layerName) == 0) {
					layer_found = true;
					break;
				}
			}
			if (!layer_found) {
				return false;
			}
		}
		return true;
	}

	std::vector<const char*> EngineDevice::getRequiredExtensions() {
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = {};
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		if (this->enabledValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}
		return extensions;
	}

	void EngineDevice::hasGLFWRequiredInstanceExtensions() {
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
		std::cout << "available extenstions: " << std::endl;

		std::unordered_set<std::string> available;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
			available.insert(extension.extensionName);
		}

		std::cout << "required extensions: " << std::endl;
		auto required_extenstions = this->getRequiredExtensions();
		for (const auto& required : required_extenstions) {
			std::cout << "\t" << required << "\n";
			if (available.find(required) == available.end()) {
				throw std::runtime_error("missing required glfw extension");
			}
		}
	}

	bool EngineDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extension_count = {};
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());
		std::set<std::string> required_extenstions(deviceExtensions_.begin(), deviceExtensions_.end());
		for (const auto& extension : available_extensions) {
			required_extenstions.erase(extension.extensionName);
		}
		auto res = required_extenstions.empty();
		return res;
	}

	QueueFamilyIndices EngineDevice::findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		uint32_t queue_family_count = {};
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
		int i = 0;
		for (const auto& queue_family : queue_families) {
			if (queue_family.queueCount > 0 && (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
				indices.graphicsFamily = i;
				indices.graphicsFamilyHasValue = true;
			}
			VkBool32 present_support = 0;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &present_support);
			if (queue_family.queueCount > 0 && present_support != 0) {
				indices.presentFamily = i;
				indices.presentFamilyHasValue = true;
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}
		return indices;
	}

	SwapChainSupportDetails EngineDevice::querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilites);
		uint32_t format_count = {};
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, nullptr);
		if (format_count != 0) {
			details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, details.formats.data());
		}

		uint32_t present_mode_count = {};
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, nullptr);
		if (present_mode_count != 0) {
			details.presentModes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, details.presentModes.data());
		}
		return details;
	}

	VkFormat EngineDevice::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (const auto format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format");
	}

	void EngineDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buf, VkDeviceMemory &buf_memory) {
		VkBufferCreateInfo buf_info {};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.size = size;
		buf_info.usage = usage;
		buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device_, &buf_info, nullptr, &buf) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer");
		}

		VkMemoryRequirements memory_reqs;
		vkGetBufferMemoryRequirements(device_, buf, &memory_reqs);
		VkMemoryAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = memory_reqs.size;
		alloc_info.memoryTypeIndex = findMemoryType(memory_reqs.memoryTypeBits, properties);
		if (vkAllocateMemory(device_, &alloc_info, nullptr, &buf_memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory");
		}

		vkBindBufferMemory(device_, buf, buf_memory, 0);
	}

	VkCommandBuffer EngineDevice::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = commandPool_;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buf = {};
		vkAllocateCommandBuffers(device_, &alloc_info, &command_buf);

		VkCommandBufferBeginInfo begin_info {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buf, &begin_info);
		return command_buf;
	}

	void EngineDevice::endSingleTimeCommands(VkCommandBuffer command_buf) {
		vkEndCommandBuffer(command_buf);
		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buf;
		vkQueueSubmit(graphicsQueue_, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue_);
		vkFreeCommandBuffers(device_, commandPool_, 1, &command_buf);
	}

	void EngineDevice::copyBuffer(VkBuffer src_buf, VkBuffer dst_buf, VkDeviceSize size) {
		VkCommandBuffer command_buf = this->beginSingleTimeCommands();
		VkBufferCopy copy_region {};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;
		vkCmdCopyBuffer(command_buf, src_buf, dst_buf, 1, &copy_region);
		endSingleTimeCommands(command_buf);
	}

	void EngineDevice::copyBufferToImage(VkBuffer buf, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count) {
		VkCommandBuffer command_buf = beginSingleTimeCommands();
		VkBufferImageCopy region {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layer_count;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };
		vkCmdCopyBufferToImage(command_buf, buf, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		endSingleTimeCommands(command_buf);
	}

	void EngineDevice::createImageWithInfo(const VkImageCreateInfo &image_info, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &image_memory) {
		if (vkCreateImage(device_, &image_info, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image");
		}
		VkMemoryRequirements memory_reqs;
		vkGetImageMemoryRequirements(device_, image, &memory_reqs);
		VkMemoryAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = memory_reqs.size;
		alloc_info.memoryTypeIndex = findMemoryType(memory_reqs.memoryTypeBits, properties);
		if (vkAllocateMemory(device_, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory");
		}
		if (vkBindImageMemory(device_, image, image_memory, 0) != VK_SUCCESS) {
			throw std::runtime_error("failed to bind image memory");
		}
	}

	uint32_t EngineDevice::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &mem_properties);
		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
			if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type");
	}

	void EngineDevice::setupDebugMessenger() {
		if (!enabledValidationLayers) {
			return;
		}
		VkDebugUtilsMessengerCreateInfoEXT create_info;
		populateDebugMessengerCreateInfo(create_info);
		if (create_debug_utils_messenger_ext(instance_, &create_info, nullptr, &debugMessenger_) != VK_SUCCESS) {
			throw std::runtime_error("failed to setup debug messenger");
		}
	}

	QueueFamilyIndices EngineDevice::findPhysicalQueueFamilies() {
		return this->findQueueFamilies(physicalDevice_);
	}

	SwapChainSupportDetails EngineDevice::swapChainSupport() {
		return querySwapChainSupport(physicalDevice_);
	}

} // namespace engine
