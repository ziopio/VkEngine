#include "stdafx.h"
#include "Mesh.h"

#include "ApiUtils.h"
#include "PhysicalDevice.h"
#include "Device.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "Libraries/tiny_obj_loader.h"



Mesh::Mesh(std::string modelPath)
{
	this->loadModel(modelPath);
	this->createVertexBuffer();
	this->createIndexBuffer();
}

VkBuffer Mesh::getVkVertexBuffer()
{
	return this->vertexBuffer;
}

VkBuffer Mesh::getVkIndexBuffer()
{
	return this->indexBuffer;
}

Mesh::~Mesh()
{
	vkDestroyBuffer(Device::get(), indexBuffer, nullptr);
	vkFreeMemory(Device::get(), indexBufferMemory, nullptr);
	vkDestroyBuffer(Device::get(), vertexBuffer, nullptr);
	vkFreeMemory(Device::get(), vertexBufferMemory, nullptr);
}

void Mesh::loadModel(std::string modelPath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath.c_str())) {
		throw std::runtime_error(err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};


	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};
			//nota attrib_t.vertices è un array di float non di vec3
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void Mesh::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	// first I create a reachable "staging buffer" as source of data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(PhysicalDevice::get(), Device::get(),
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	// Mappo la memoria VRAM in RAM puntata da "data"
	void* data;
	vkMapMemory(Device::get(), stagingBufferMemory, 0, bufferSize, 0, &data);
	// Trasgferisco i dati sulla memoria mappata (stage bufffer filling)
	memcpy(data, vertices.data(), (size_t)bufferSize);
	// Disabilito la mappatura
	vkUnmapMemory(Device::get(), stagingBufferMemory);

	// create the effective optimized vertex buffer to destinate data
	createBuffer(PhysicalDevice::get(), Device::get(),
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory);
	copyBufferToBuffer(Device::get(),Device::getGraphicQueue(),Device::getGraphicCmdPool(),stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(Device::get(), stagingBuffer, nullptr);
	vkFreeMemory(Device::get(), stagingBufferMemory, nullptr);
}

void Mesh::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(PhysicalDevice::get(), Device::get(),
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(Device::get(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(Device::get(), stagingBufferMemory);

	createBuffer(PhysicalDevice::get(), Device::get(),
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		indexBuffer, indexBufferMemory);

	copyBufferToBuffer(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(), stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(Device::get(), stagingBuffer, nullptr);
	vkFreeMemory(Device::get(), stagingBufferMemory, nullptr);
}
