#include "buffer.hpp"

#include <cassert>
#include <cstring>

namespace engine {

	VkDeviceSize Buffer::alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) {
		if (min_offset_alignment > 0) {
			return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
		}
		return instance_size;
	}

	Buffer::Buffer(
		EngineDevice& device,
		VkDeviceSize instance_size,
		uint32_t instance_count,
		VkBufferUsageFlags usage_flags,
		VkMemoryPropertyFlags memory_property_flags,
		VkDeviceSize min_offset_alignment
	) :
		device_ { device },
		instanceCount_ { instance_count },
		instanceSize_ { instance_size },
		alignmentSize_ { alignment(instance_size, min_offset_alignment) },
		usageFlags_ { usage_flags },
		memoryPropertyFlags_ { memory_property_flags } {
			bufferSize_ = alignmentSize_ * instanceCount_;
			device_.createBuffer(bufferSize_, usageFlags_, memoryPropertyFlags_, buffer_, memory_);
	}

	Buffer::~Buffer() {
		unmap();
		vkDestroyBuffer(device_.device(), buffer_, nullptr);
		vkFreeMemory(device_.device(), memory_, nullptr);
	}


	VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
		assert(buffer_ && memory_ && "Called map on buffer before create"); // NOLINT
		return vkMapMemory(device_.device(), memory_, offset, size, 0, &mapped_);
	}

	void Buffer::unmap() {
		if (mapped_ != nullptr) {
			vkUnmapMemory(device_.device(), memory_);
			mapped_ = nullptr;
		}
	}

	void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
		assert(mapped_ && "Cannot copy to unmapped buffer"); // NOLINT
		if (size == VK_WHOLE_SIZE) {
			memcpy(mapped_, data, bufferSize_);
		} else {
			char* mem_offset = static_cast<char*>(mapped_);
			mem_offset += offset;
			memcpy(mem_offset, data, size);
		}
	}

	VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
		VkMappedMemoryRange mapped_range {};
		mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mapped_range.memory = memory_;
		mapped_range.offset = offset;
		mapped_range.size = size;
		return vkFlushMappedMemoryRanges(device_.device(), 1, &mapped_range);
	}

	VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
		VkMappedMemoryRange mapped_range {};
		mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mapped_range.memory = memory_;
		mapped_range.offset = offset;
		mapped_range.size = size;
		return vkInvalidateMappedMemoryRanges(device_.device(), 1, &mapped_range);
	}

	VkDescriptorBufferInfo Buffer::decriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
		return VkDescriptorBufferInfo { buffer_, offset, size };
	}

	void Buffer::writeToIndex(void* data, int index) {
		writeToBuffer(data, instanceSize_, index * alignmentSize_);
	}

	VkResult Buffer::flushIndex(int index) {
		return flush(alignmentSize_, index * alignmentSize_);
	}

	VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
		return decriptorInfo(alignmentSize_, index * alignmentSize_);
	}

	VkResult Buffer::invalidateIndex(int index) {
		return invalidate(alignmentSize_, index * alignmentSize_);
	}


}
