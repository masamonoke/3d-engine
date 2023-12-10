#include "model.hpp"

#include <cassert>

namespace engine {

	Model::Model(EngineDevice& device, const Builder& builder) : device_{device} {
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	Model::~Model() {
		vkDestroyBuffer(device_.device(), vertexBuffer_, nullptr);
		vkFreeMemory(device_.device(), vertexBufferMemory_, nullptr);
		if (hasIndexBuffer_) {
			vkDestroyBuffer(device_.device(), indexBuffer_, nullptr);
			vkFreeMemory(device_.device(), indexBufferMemory_, nullptr);
		}
	}

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount_ = static_cast<uint32_t>(vertices.size());
		assert(vertexCount_ >= 3);
		VkDeviceSize buf_size = sizeof(vertices[0]) * vertexCount_;
		VkBuffer staging_buf;
		VkDeviceMemory staging_buf_memory;
		device_.createBuffer(buf_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buf,
			staging_buf_memory
		);

		void* data;
		vkMapMemory(device_.device(), staging_buf_memory, 0, buf_size, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(buf_size));
		vkUnmapMemory(device_.device(), staging_buf_memory);

		device_.createBuffer(buf_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer_,
			vertexBufferMemory_
		);
		device_.copyBuffer(staging_buf, vertexBuffer_, buf_size);
		vkDestroyBuffer(device_.device(), staging_buf, nullptr);
		vkFreeMemory(device_.device(), staging_buf_memory, nullptr);
	}

	void Model::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount_ = static_cast<uint32_t>(indices.size());
		hasIndexBuffer_ = indexCount_ > 0;
		if (!hasIndexBuffer_) {
			return;
		}
		VkDeviceSize buf_size = sizeof(indices[0]) * indexCount_;
		VkBuffer staging_buf;
		VkDeviceMemory staging_buf_memory;
		device_.createBuffer(buf_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buf,
			staging_buf_memory
		);

		void* data;
		vkMapMemory(device_.device(), staging_buf_memory, 0, buf_size, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(buf_size));
		vkUnmapMemory(device_.device(), staging_buf_memory);

		device_.createBuffer(buf_size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer_,
			indexBufferMemory_
		);
		device_.copyBuffer(staging_buf, indexBuffer_, buf_size);
		vkDestroyBuffer(device_.device(), staging_buf, nullptr);
		vkFreeMemory(device_.device(), staging_buf_memory, nullptr);
}

	void Model::bind(VkCommandBuffer command_buf) {
		VkBuffer buffers[] = { vertexBuffer_ };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buf, 0, 1, buffers, offsets);
		if (hasIndexBuffer_) {
			vkCmdBindIndexBuffer(command_buf, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void Model::draw(VkCommandBuffer command_buf) {
		if (hasIndexBuffer_) {
			vkCmdDrawIndexed(command_buf, indexCount_, 1, 0, 0, 0);
		} else {
			vkCmdDraw(command_buf, vertexCount_, 1, 0, 0);
		}
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
		attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(Vertex, position);
		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(Vertex, color);

		return attribute_descriptions;
	}
}
