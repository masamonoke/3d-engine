#include "pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

#include "utils.hpp"
#include "model.hpp"

namespace engine {

	Pipeline::Pipeline( // NOLINT
		EngineDevice& device,
		const std::string& vert_path,
		const std::string& frag_path,
		const PipelineConfigInfo& config_info) : device_(device) {
		createGraphicsPipeline(vert_path, frag_path, config_info);
	}

	Pipeline::~Pipeline() {
		vkDestroyShaderModule(device_.device(), vertShaderModule_, nullptr);
		vkDestroyShaderModule(device_.device(), fragShaderModule_, nullptr);
		vkDestroyPipeline(device_.device(), graphicsPipeline_, nullptr);
	}

	std::vector<char> Pipeline::readFile(const std::string& filepath) {
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filepath);
		}
		auto size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(size);
		file.seekg(0);
		file.read(buffer.data(), size); // NOLINT
		file.close();
		return buffer;
	}

	void Pipeline::createGraphicsPipeline(const std::string& vert_path, const std::string& frag_path, const PipelineConfigInfo& config_info) {
		assert(config_info.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided"); // NOLINT
		assert(config_info.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no render pass provided"); // NOLINT
		auto vert_code = readFile(vert_path);
		auto frag_code = readFile(frag_path);
		createShaderModule(vert_code, &vertShaderModule_);
		createShaderModule(frag_code, &fragShaderModule_);

		std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {};
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = vertShaderModule_;
		shader_stages[0].pName = "main";
		shader_stages[0].flags = 0;
		shader_stages[0].pNext = nullptr;
		shader_stages[0].pSpecializationInfo = nullptr;
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = fragShaderModule_;
		shader_stages[1].pName = "main";
		shader_stages[1].flags = 0;
		shader_stages[1].pNext = nullptr;
		shader_stages[1].pSpecializationInfo = nullptr;

		/* auto binding_descriptions = Model::Vertex::getBindingDescriptions(); */
		/* auto attribute_descriptions = Model::Vertex::getAttributeDescriptions(); */
		const auto& binding_descriptions = config_info.bindingDescriptions;
		const auto& attribute_descriptions = config_info.attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo vertex_input_info {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
		vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
		vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
		vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

		VkGraphicsPipelineCreateInfo pipeline_info {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &config_info.inputAssemblyInfo;
		pipeline_info.pViewportState = &config_info.viewportInfo;
		pipeline_info.pRasterizationState = &config_info.rasterizationInfo;
		pipeline_info.pMultisampleState = &config_info.multisampleInfo;
		pipeline_info.pColorBlendState = &config_info.colorBlendInfo;
		pipeline_info.pDepthStencilState = &config_info.depthStencilInfo;
		pipeline_info.pDynamicState = &config_info.dynamamicStateInfo;

		pipeline_info.layout = config_info.pipelineLayout;
		pipeline_info.renderPass = config_info.renderPass;
		pipeline_info.subpass = config_info.subpass;
		pipeline_info.basePipelineIndex = -1;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;


		if (vkCreateGraphicsPipelines(device_.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphicsPipeline_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline");
		}
	}

	void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module) {
		VkShaderModuleCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = code.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
		if (vkCreateShaderModule(device_.device(), &create_info, nullptr, shader_module) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
	}

	PipelineConfigInfo Pipeline::defaultPipelineConfigInfo() {
		PipelineConfigInfo config_info {};
		config_info.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		config_info.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config_info.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		config_info.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		config_info.viewportInfo.viewportCount = 1;
		config_info.viewportInfo.pViewports = nullptr;
		config_info.viewportInfo.scissorCount = 1;
		config_info.viewportInfo.pScissors = nullptr;

		config_info.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		config_info.rasterizationInfo.depthClampEnable = VK_FALSE;
		config_info.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		config_info.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		config_info.rasterizationInfo.lineWidth = 1.0F;
		config_info.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		config_info.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		config_info.rasterizationInfo.depthBiasEnable = VK_FALSE;
		config_info.rasterizationInfo.depthBiasConstantFactor = 0.0F;
		config_info.rasterizationInfo.depthBiasClamp = 0.0F;
		config_info.rasterizationInfo.depthBiasSlopeFactor = 0.0F;

		config_info.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		config_info.multisampleInfo.sampleShadingEnable = VK_FALSE;
		config_info.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		config_info.multisampleInfo.minSampleShading = 1.0F;
		config_info.multisampleInfo.pSampleMask = nullptr;
		config_info.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		config_info.multisampleInfo.alphaToOneEnable = VK_FALSE;

		config_info.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		config_info.colorBlendAttachment.blendEnable = VK_FALSE;
		config_info.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		config_info.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		config_info.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		config_info.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		config_info.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		config_info.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		config_info.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		config_info.colorBlendInfo.logicOpEnable = VK_FALSE;
		config_info.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
		config_info.colorBlendInfo.attachmentCount = 1;
		config_info.colorBlendInfo.pAttachments = &config_info.colorBlendAttachment;
		config_info.colorBlendInfo.blendConstants[0] = 0.0F;
		config_info.colorBlendInfo.blendConstants[1] = 0.0F;
		config_info.colorBlendInfo.blendConstants[2] = 0.0F;
		config_info.colorBlendInfo.blendConstants[3] = 0.0F;

		config_info.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		config_info.depthStencilInfo.depthTestEnable = VK_TRUE;
		config_info.depthStencilInfo.depthWriteEnable = VK_TRUE;
		config_info.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		config_info.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		config_info.depthStencilInfo.minDepthBounds = 0.0F;
		config_info.depthStencilInfo.maxDepthBounds = 1.0F;
		config_info.depthStencilInfo.stencilTestEnable = VK_FALSE;
		config_info.depthStencilInfo.front = {};
		config_info.depthStencilInfo.back = {};

		config_info.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		config_info.dynamamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		config_info.dynamamicStateInfo.pDynamicStates = config_info.dynamicStateEnables.data();
		config_info.dynamamicStateInfo.dynamicStateCount = static_cast<uint32_t>(config_info.dynamicStateEnables.size());
		config_info.dynamamicStateInfo.flags = 0;

		config_info.bindingDescriptions = Model::Vertex::getBindingDescriptions();
		config_info.attributeDescriptions = Model::Vertex::getAttributeDescriptions();

		return config_info;
	}

	void Pipeline::bind(VkCommandBuffer command_buffer) {
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
	}
}
