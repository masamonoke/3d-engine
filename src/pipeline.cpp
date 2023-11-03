#include "pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

#include "utils.hpp"

namespace engine {

	Pipeline::Pipeline(EngineDevice& device, const std::string& vert_path, const std::string& frag_path, const PipelineConfigInfo& config_info) : device_(device) {
		createGraphicsPipeline(vert_path, frag_path, config_info);
	}

	std::vector<char> Pipeline::readFile(const std::string& filepath) {
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filepath);
		}
		auto size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(size);
		file.seekg(0);
		file.read(buffer.data(), size);
		file.close();
		return buffer;
	}

	void Pipeline::createGraphicsPipeline(const std::string& vert_path, const std::string& frag_path, const PipelineConfigInfo& config_info) {
		UNUSED(config_info);
		auto vert_code = readFile(vert_path);
		auto frag_code = readFile(frag_path);
		std::cout << "vertext shader code size: " << vert_code.size() << "\n";
		std::cout << "fragment shader code size: " << frag_code.size() << std::endl;
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

	PipelineConfigInfo Pipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
		UNUSED(width);
		UNUSED(height);
		PipelineConfigInfo config_info {};
		return config_info;
	}

}
