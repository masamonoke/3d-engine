#include "model.hpp"

#include <cassert>

namespace engine {

	Model::Model(EngineDevice& device, const std::vector<Vertex>& vertices) : device_{device} {
		createVertexBuffers(vertices);
	}

	Model::~Model() {
		vkDestroyBuffer(device_.device(), vertexBuffer_, nullptr);
		vkFreeMemory(device_.device(), vertexBufferMemory_, nullptr);
	}

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount_ = static_cast<uint32_t>(vertices.size());
		assert(vertexCount_ >= 3);
		VkDeviceSize buf_size = sizeof(vertices[0]) * vertexCount_;
		device_.createBuffer(buf_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer_,
			vertexBufferMemory_
		);
		void* data;
		vkMapMemory(device_.device(), vertexBufferMemory_, 0, buf_size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(buf_size));
		vkUnmapMemory(device_.device(), vertexBufferMemory_);
	}

	void Model::bind(VkCommandBuffer command_buf) {
		VkBuffer buffers[] = { vertexBuffer_ };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buf, 0, 1, buffers, offsets);
	}

	void Model::draw(VkCommandBuffer command_buf) {
		vkCmdDraw(command_buf, vertexCount_, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
		binding_descriptions[0].binding = 0;
		binding_descriptions[0].stride = sizeof(Vertex);
		binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(Vertex, position);
		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(Vertex, color);

		return attribute_descriptions;
	}
}
