#include "model.hpp"
#include "utils.hpp"

#define TINYOBJLEADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <iostream>
#include <unordered_map>

namespace std {
	template<>
	struct hash<engine::Model::Vertex> {
		size_t operator()(engine::Model::Vertex const & vertex) const {
			size_t seed = 0;
			engine::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace engine {

	Model::Model(EngineDevice& device, const Builder& builder) : device_ { device }, vertexCount_ { 0 }, indexCount_ { 0 } {
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	Model::~Model() = default;

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount_ = static_cast<uint32_t>(vertices.size());
		assert(vertexCount_ >= 3);
		const VkDeviceSize buf_size = sizeof(vertices[0]) * vertexCount_;
		const uint32_t vertex_size = sizeof(vertices[0]);


		Buffer staging_buf = {
			device_,
			vertex_size,
			vertexCount_,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		staging_buf.map();
		staging_buf.writeToBuffer((void*) vertices.data());


		vertexBuffer_ = std::make_unique<Buffer>(
			device_,
			vertex_size,
			vertexCount_,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device_.copyBuffer(staging_buf.buffer(), vertexBuffer_->buffer(), buf_size);
	}

	void Model::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount_ = static_cast<uint32_t>(indices.size());
		hasIndexBuffer_ = indexCount_ > 0;
		if (!hasIndexBuffer_) {
			return;
		}
		const VkDeviceSize buf_size = sizeof(indices[0]) * indexCount_;

		const uint32_t index_size = sizeof(indices[0]);
		Buffer staging_buf = {
			device_,
			index_size,
			indexCount_,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		staging_buf.map();
		staging_buf.writeToBuffer((void*)indices.data());

		indexBuffer_ = std::make_unique<Buffer>(
			device_,
			index_size,
			indexCount_,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		device_.copyBuffer(staging_buf.buffer(), indexBuffer_->buffer(), buf_size);
}

	void Model::bind(VkCommandBuffer command_buf) {
		std::vector<VkBuffer> buffers { vertexBuffer_->buffer() };
		std::vector<VkDeviceSize> offsets { 0 };
		vkCmdBindVertexBuffers(command_buf, 0, 1, buffers.data(), offsets.data());
		if (hasIndexBuffer_) {
			vkCmdBindIndexBuffer(command_buf, indexBuffer_->buffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void Model::draw(VkCommandBuffer command_buf) const {
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
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions {};

		attribute_descriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attribute_descriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attribute_descriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attribute_descriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

		return attribute_descriptions;
	}

	std::unique_ptr<Model> Model::createModelFromFile(EngineDevice& device, const std::string& filepath) {
		Builder builder {};
		builder.loadModel(filepath);
		std::cout << "Vertex count: " << builder.vertices.size() << "\n";
		return std::make_unique<Model>(device, builder);
	}

	void Model::Builder::loadModel(const std::string& filepath) {
		tinyobj::attrib_t attr;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;
		if (!tinyobj::LoadObj(&attr, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}
		this->vertices.clear();
		this->indices.clear();
		std::unordered_map<Vertex, uint32_t> unique_vertices {};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex {};
				if (index.vertex_index >= 0) {
					vertex.position = {
						attr.vertices[3 * index.vertex_index + 0],
						attr.vertices[3 * index.vertex_index + 1],
						attr.vertices[3 * index.vertex_index + 2]
					};

					vertex.color = {
						attr.colors[3 * index.vertex_index + 0],
						attr.colors[3 * index.vertex_index + 1],
						attr.colors[3 * index.vertex_index + 2]
					};
				}
				if (index.normal_index >= 0) {
					vertex.normal = {
						attr.normals[3 * index.normal_index + 0],
						attr.normals[3 * index.normal_index + 1],
						attr.normals[3 * index.normal_index + 2]
					};
				}
				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attr.texcoords[2 * index.texcoord_index + 0],
						attr.texcoords[2 * index.texcoord_index + 1],
					};
				}
				if (!unique_vertices.contains(vertex)) {
					unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(unique_vertices[vertex]);
			}
		}
	}

}
