#ifndef MODEL_HPP
#define MODEL_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

#include "engine_device.hpp"

namespace engine {
	class Model {
		public:
			struct Vertex {
				glm::vec3 position;
				glm::vec3 color;

				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			};

			Model(EngineDevice& device, const std::vector<Vertex>& vertices);
			~Model();

			Model(const Model&) = delete;
			Model& operator=(const Model&) = delete;

			void bind(VkCommandBuffer command_buf);
			void draw(VkCommandBuffer command_buf);

		private:
			EngineDevice& device_;
			VkBuffer vertexBuffer_;
			VkDeviceMemory vertexBufferMemory_;
			uint32_t vertexCount_;

			void createVertexBuffers(const std::vector<Vertex>& vertices);
	};
}

#endif // MODEL_HPP
