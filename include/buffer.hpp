#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "engine_device.hpp"

namespace engine {

	class Buffer {
		public:
			Buffer(
				EngineDevice& device,
				VkDeviceSize instance_size,
				uint32_t instance_count,
				VkBufferUsageFlags usage_flags,
				VkMemoryPropertyFlags memory_property_flags,
				VkDeviceSize min_offset_alignment = 1
			);
			~Buffer();
			Buffer(const Buffer&) = delete;
			Buffer& operator=(const Buffer&) = delete;

			VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void unmap();
			void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			VkDescriptorBufferInfo decriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
			void writeToIndex(void* data, int index);
			VkResult flushIndex(int index);
			VkDescriptorBufferInfo descriptorInfoForIndex(int index);
			VkResult invalidateIndex(int index);

			[[nodiscard]] VkBuffer buffer() const { return buffer_; }
			[[nodiscard]] void* mappedMemory() const { return mapped_; }
			[[nodiscard]] uint32_t instanceCount() const { return instanceCount_; }
			[[nodiscard]] VkDeviceSize instanceSize() const { return instanceSize_; }
			[[nodiscard]] VkDeviceSize alignmentSize() const { return alignmentSize_; }
			[[nodiscard]] VkBufferUsageFlags usageFlags() const { return usageFlags_; }
			[[nodiscard]] VkMemoryPropertyFlags memoryPropertyFlags() const { return memoryPropertyFlags_; }
			[[nodiscard]] VkDeviceSize bufferSize() const { return bufferSize_; }


		private:
			EngineDevice& device_;
			void* mapped_ = nullptr;
			VkBuffer buffer_ = VK_NULL_HANDLE;
			VkDeviceMemory memory_ = VK_NULL_HANDLE;
			VkDeviceSize bufferSize_;
			uint32_t instanceCount_;
			VkDeviceSize instanceSize_;
			VkDeviceSize alignmentSize_;
			VkBufferUsageFlags usageFlags_;
			VkMemoryPropertyFlags memoryPropertyFlags_;

			static VkDeviceSize alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);
	};

}

#endif // BUFFER_HPP
