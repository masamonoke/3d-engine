#include "descriptor.hpp"

#include <cassert>
#include <stdexcept>

namespace engine {

	// DescriptorSetLayout::Builder

	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
			uint32_t binding,
			VkDescriptorType desc_type,
			VkShaderStageFlags stage_flags, // NOLINT
			uint32_t count) {
		assert(bindings_.count(binding) == 0 && "Binding already in use"); // NOLINT

		VkDescriptorSetLayoutBinding layout_binding {};
		layout_binding.binding = binding;
		layout_binding.descriptorType = desc_type;
		layout_binding.descriptorCount = count;
		layout_binding.stageFlags = stage_flags;
		bindings_[binding] = layout_binding;
		return *this;
	}

	std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
		return std::make_unique<DescriptorSetLayout>(device_, bindings_);
	}

	//DescriptorSetLayout

	DescriptorSetLayout::DescriptorSetLayout( // NOLINT
		EngineDevice& device,
		const std::unordered_map<uint32_t,
		VkDescriptorSetLayoutBinding>& bindings) : device_ { device }, bindings_ { bindings } {

		std::vector<VkDescriptorSetLayoutBinding> layout_binding_set { bindings.size() - 1 };
		for (auto key_value : bindings) {
			layout_binding_set.push_back(key_value.second);
		}
		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info {};
		descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(layout_binding_set.size());
		descriptor_set_layout_info.pBindings = layout_binding_set.data();
		if (vkCreateDescriptorSetLayout(device_.device(), &descriptor_set_layout_info, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout");
		}
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		vkDestroyDescriptorSetLayout(device_.device(), descriptorSetLayout_, nullptr);
	}

	// DescriptorPool::Builder

	DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType desc_type, uint32_t count) {
		poolSizes_.push_back({ desc_type, count });
		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::poolFlags(VkDescriptorPoolCreateFlags flags) {
		poolFlags_ = flags;
		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::maxSets(uint32_t count) {
		maxSets_ = count;
		return *this;
	}

	std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
		return std::make_unique<DescriptorPool>(device_, maxSets_, poolFlags_, poolSizes_);
	}

	// DescriptorPool

	DescriptorPool::DescriptorPool(EngineDevice& device, // NOLINT
		uint32_t max_sets, // NOLINT
		VkDescriptorPoolCreateFlags pool_flags,
		const std::vector<VkDescriptorPoolSize>& pool_sizes) : device_ { device } {
		VkDescriptorPoolCreateInfo descriptor_pool_info {};
		descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		descriptor_pool_info.pPoolSizes = pool_sizes.data();
		descriptor_pool_info.maxSets = max_sets;
		descriptor_pool_info.flags = pool_flags;
		if (vkCreateDescriptorPool(device_.device(), &descriptor_pool_info, nullptr, &descriptorPool_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool");
		}
	}

	DescriptorPool::~DescriptorPool() {
		vkDestroyDescriptorPool(device_.device(), descriptorPool_, nullptr);
	}

	bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout desc_set_layout, VkDescriptorSet &desc) const {
		VkDescriptorSetAllocateInfo alloc_info {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = descriptorPool_;
		alloc_info.pSetLayouts = &desc_set_layout;
		alloc_info.descriptorSetCount = 1;
		/* if (vkAllocateDescriptorSets(device_.device(), &alloc_info, &desc) != VK_SUCCESS) { */
		/* 	return false; */
		/* } */
		/* return true; */
		return vkAllocateDescriptorSets(device_.device(), &alloc_info, &desc) == VK_SUCCESS;
	}

	void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
		vkFreeDescriptorSets(device_.device(), descriptorPool_, static_cast<uint32_t>(descriptors.size()), descriptors.data());
	}

	void DescriptorPool::resetPool() {
		vkResetDescriptorPool(device_.device(), descriptorPool_, 0);
	}

	// DescriptorWriter

	DescriptorWriter::DescriptorWriter(DescriptorSetLayout& set_layout, DescriptorPool& pool) : setLayout_ { set_layout }, pool_ { pool } {}

	DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) {
		assert(setLayout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");
		auto& binding_description = setLayout_.bindings_[binding];
		assert(binding_description.descriptorCount == 1 && "Binding single descriptor info but bindingCount expects multiple");
		VkWriteDescriptorSet write {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = binding_description.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = buffer_info;
		write.descriptorCount = 1;
		writes_.push_back(write);
		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *image_info) {
		assert(setLayout_.bindings_.count(binding) == 1 && "Layout does not contain specified binding");
		auto& binding_description = setLayout_.bindings_[binding];
		assert(binding_description.descriptorCount == 1 && "Binding single descriptor info but bindingCount expects multiple");
		VkWriteDescriptorSet write {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = binding_description.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = image_info;
		write.descriptorCount = 1;
		writes_.push_back(write);
		return *this;
	}

	bool DescriptorWriter::build(VkDescriptorSet &set) {
		const bool success = pool_.allocateDescriptor(setLayout_.descriptorSetLayout(), set);
		if (!success) {
			return false;
		}
		overwrite(set);
		return true;
	}

	void DescriptorWriter::overwrite(VkDescriptorSet &set) {
		for (auto& write : writes_) {
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(pool_.device_.device(), writes_.size(), writes_.data(), 0, nullptr);
	}

}
