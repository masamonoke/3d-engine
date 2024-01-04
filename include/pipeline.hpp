#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <string>

#include "engine_device.hpp"

namespace engine {

	struct PipelineConfigInfo {
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamamicStateInfo;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions {};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions {};
	};

	class Pipeline {
		public:
			Pipeline(EngineDevice& device, const std::string& vert_path, const std::string& frag_path, const PipelineConfigInfo& config_info);
			~Pipeline();
			Pipeline(const Pipeline&) = delete;
			Pipeline& operator=(const Pipeline&) = delete;
			Pipeline(const Pipeline&&) = delete;
			Pipeline&& operator=(const Pipeline&&) = delete;

			static PipelineConfigInfo defaultPipelineConfigInfo();
			void bind(VkCommandBuffer command_buffer);

		private:
			EngineDevice& device_;
			VkPipeline graphicsPipeline_;
			VkShaderModule vertShaderModule_;
			VkShaderModule fragShaderModule_;

			static std::vector<char> readFile(const std::string& filepath);
			void createGraphicsPipeline(const std::string& vert_path, const std::string& frag_path, const PipelineConfigInfo& config_info);
			void createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);
	};
}

#endif // PIPELINE_HPP
