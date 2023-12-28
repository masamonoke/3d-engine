#ifndef MODEL_HPP
#define MODEL_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

#include "engine_device.hpp"
#include "buffer.hpp"

namespace engine {

	class Model {
		public:
			struct Vertex {
				glm::vec3 position {};
				glm::vec3 color {};
				glm::vec3 normal {};
				glm::vec2 uv {};

				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

				bool operator==(const Vertex& other) const {
					return this->position == other.position && this->color == other.color && this->normal == other.normal && this->uv == other.uv;
				}
			};

			struct Builder {
				std::vector<Vertex> vertices {};
				std::vector<uint32_t> indices {};

				void loadModel(const std::string &filepath);
			};

			Model(EngineDevice& device, const Builder& builder);
			~Model();

			Model(const Model&) = delete;
			Model& operator=(const Model&) = delete;

			void bind(VkCommandBuffer command_buf);
			void draw(VkCommandBuffer command_buf) const;

			static std::unique_ptr<Model> createModelFromFile(EngineDevice& device, const std::string& filepath);

		private:
			EngineDevice& device_;

			std::unique_ptr<Buffer> vertexBuffer_;
			uint32_t vertexCount_;

			bool hasIndexBuffer_ = false;
			std::unique_ptr<Buffer> indexBuffer_;
			uint32_t indexCount_;

			void createVertexBuffers(const std::vector<Vertex>& vertices);
			void createIndexBuffers(const std::vector<uint32_t>& indices);

	};

}

#endif // MODEL_HPP
