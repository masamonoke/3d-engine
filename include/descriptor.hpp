#ifndef DESCRIPTOR_HPP
#define DESCRIPTOR_HPP

#include <memory>
#include <unordered_map>
#include <vector>

#include "engine_device.hpp"

namespace engine {

	class DescriptorSetLayout {
		public:
			class Builder {
				public:
					explicit Builder(EngineDevice& device) : device_ { device } {}

					Builder& addBinding(uint32_t binding, VkDescriptorType desc_type, VkShaderStageFlags stage_flags, uint32_t count = 1);
					[[nodiscard]] std::unique_ptr<DescriptorSetLayout> build() const;

				private:
					EngineDevice& device_; // NOLINT
					std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_ {};

			};

			DescriptorSetLayout(EngineDevice& device, const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);
			~DescriptorSetLayout();
			DescriptorSetLayout(const DescriptorSetLayout&) = delete;
			DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
			DescriptorSetLayout(const DescriptorSetLayout&&) = delete;
			DescriptorSetLayout&& operator=(const DescriptorSetLayout&&) = delete;

			[[nodiscard]] VkDescriptorSetLayout descriptorSetLayout() const { return descriptorSetLayout_; }

			private:
				EngineDevice& device_; // NOLINT
				VkDescriptorSetLayout descriptorSetLayout_;
				std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;
				friend class DescriptorWriter;
	};

	const uint32_t MAX_SETS = 1000;

	class DescriptorPool {
		public:
			class Builder {
				public:
					explicit Builder(EngineDevice& device) : device_ { device } {}

					Builder& addPoolSize(VkDescriptorType desc_type, uint32_t count);
					Builder& poolFlags(VkDescriptorPoolCreateFlags flags);
					Builder& maxSets(uint32_t count);
					[[nodiscard]] std::unique_ptr<DescriptorPool> build() const;

				private:
					EngineDevice& device_; // NOLINT
					std::vector<VkDescriptorPoolSize> poolSizes_ {};
					uint32_t maxSets_ = MAX_SETS;
					VkDescriptorPoolCreateFlags poolFlags_ = 0;
			};

			DescriptorPool(EngineDevice& device, uint32_t max_sets, VkDescriptorPoolCreateFlags pool_flags, const std::vector<VkDescriptorPoolSize>& pool_sizes);
			~DescriptorPool();
			DescriptorPool(const DescriptorPool&) = delete;
			DescriptorPool& operator=(const DescriptorPool&) = delete;
			DescriptorPool(const DescriptorPool&&) = delete;
			DescriptorPool&& operator=(const DescriptorPool&&) = delete;

			bool allocateDescriptor(const VkDescriptorSetLayout desc_set_layout, VkDescriptorSet& desc) const;
			void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
			void resetPool();

		private:
			EngineDevice& device_; // NOLINT
			VkDescriptorPool descriptorPool_;
			friend class DescriptorWriter;
	};

	class DescriptorWriter {
		public:
			DescriptorWriter(DescriptorSetLayout& set_layout, DescriptorPool& pool);
			DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info);
			DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* image_info);

			bool build(VkDescriptorSet& set);
			void overwrite(VkDescriptorSet& set);

		private:
			DescriptorSetLayout& setLayout_;
			DescriptorPool& pool_;
			std::vector<VkWriteDescriptorSet> writes_;
	};

}

#endif // DESCRIPTOR_HPP
