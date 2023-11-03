#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <string>

#include "engine_device.hpp"

namespace engine {

	struct PipelineConfigInfo {

	};

	class Pipeline {
		public:
			Pipeline(EngineDevice& device, const std::string& vert_path, const std::string& frag_path, const PipelineConfigInfo& config_info);
			~Pipeline() {}
			Pipeline(const Pipeline&) = delete;
			void operator=(const Pipeline&) = delete;
			static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

		private:
			EngineDevice& device_;
			/* VkPipeline graphicsPipeline_; */
			/* VkShaderModule vertShaderModule_; */
			/* VkShaderModule fragShaderModule_; */

			static std::vector<char> readFile(const std::string& filepath);
			void createGraphicsPipeline(const std::string& vert_path, const std::string& frag_path, const PipelineConfigInfo& config_info);
			void createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);
	};
}

#endif // PIPELINE_HPP
