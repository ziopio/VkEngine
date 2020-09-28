#include "commons.h"
#include "Mesh.h"
#include "VkEngine.h"
#include "ApiUtils.h"
#include "PhysicalDevice.h"
#include "Device.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "Libraries/tiny_obj_loader.h"


VkBuffer BaseMesh::getVkVertexBuffer() const {return this->vertexBuffer;}

VkBuffer BaseMesh::getVkIndexBuffer() const {return this->indexBuffer;}

//Mesh3D::Mesh3D(Primitive3D primitive)
//{
//	this->vertices = primitive.vertices;
//	this->indices = primitive.indices;
//	this->createVertexBuffer();
//	this->createIndexBuffer();
//}

Mesh3D::Mesh3D(std::string modelPath)
{
	this->loadModel(modelPath);
	this->createVertexBuffer();
	this->createIndexBuffer();
}

uint32_t Mesh3D::getIdxCount() const
{
	return static_cast<uint32_t>(this->indices.size());
}

uint32_t Mesh3D::getVertexCount() const
{
	return static_cast<uint32_t>(this->vertices.size());
}


Mesh3D::~Mesh3D()
{
	vkDestroyBuffer(Device::get(), indexBuffer, nullptr);
	vkFreeMemory(Device::get(), indexBufferMemory, nullptr);
	vkDestroyBuffer(Device::get(), vertexBuffer, nullptr);
	vkFreeMemory(Device::get(), vertexBufferMemory, nullptr);
}

void Mesh3D::loadModel(std::string modelPath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath.c_str())) {
		throw std::runtime_error(err);
	}

	std::unordered_map<Vertex3D, uint32_t> uniqueVertices = {};
	int i = 0;
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex3D vertex = {};
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
			//this->vertices.push_back(vertex);
			//indices.push_back(i++);
		}
	}
}

void Mesh3D::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex3D) * vertices.size();
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
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | (PhysicalDevice::hasRaytracing() ? VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR : 0),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer, vertexBufferMemory);
	copyBufferToBuffer(Device::get(),Device::getGraphicQueue(),Device::getGraphicCmdPool(),stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(Device::get(), stagingBuffer, nullptr);
	vkFreeMemory(Device::get(), stagingBufferMemory, nullptr);
}

void Mesh3D::createIndexBuffer()
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
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | (PhysicalDevice::hasRaytracing() ? VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR : 0),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer, indexBufferMemory);

	copyBufferToBuffer(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(), stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(Device::get(), stagingBuffer, nullptr);
	vkFreeMemory(Device::get(), stagingBufferMemory, nullptr);
}

GuiMesh::GuiMesh()
{
}

void GuiMesh::updateMeshData(UiDrawData draw_data)
{
	// Check if vertexBufferMemory needs reallocation
	size_t updateVtxSize = sizeof(Vertex2D) * draw_data.totalVtxCount;
	if (this->allocated_Vtx_MemSize < updateVtxSize) {
		if (allocated_Vtx_MemSize > 0) {
			vkUnmapMemory(Device::get(), vertexBufferMemory);
			vkDestroyBuffer(Device::get(), vertexBuffer, nullptr);
			vkFreeMemory(Device::get(), vertexBufferMemory, nullptr);
		}
		// double the previus allocated memory to minimize future reallocations
		this->allocated_Vtx_MemSize = updateVtxSize * 2;
		createBuffer(PhysicalDevice::get(), Device::get(),
			allocated_Vtx_MemSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer, vertexBufferMemory);
		vkMapMemory(Device::get(), vertexBufferMemory, 0, allocated_Vtx_MemSize, 0, (void**)&mappedVtxMemory);
	}
	// Check if indexBufferMemory needs reallocation
	size_t updateIdxSize = sizeof(uint32_t) * draw_data.totalIdxCount;
	if (this->allocated_Idx_MemSize < updateIdxSize) {
		if (allocated_Idx_MemSize > 0) {
			vkUnmapMemory(Device::get(), indexBufferMemory);
			vkDestroyBuffer(Device::get(), indexBuffer, nullptr);
			vkFreeMemory(Device::get(), indexBufferMemory, nullptr);
		}
		// double the previus allocated memory to minimize future reallocations
		this->allocated_Idx_MemSize = updateIdxSize * 2;
		createBuffer(PhysicalDevice::get(), Device::get(),
			allocated_Idx_MemSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			indexBuffer, indexBufferMemory);
		vkMapMemory(Device::get(), indexBufferMemory, 0, allocated_Idx_MemSize, 0, (void**)&mappedIdxMemory);
	}
	//Update vertex and index buffer with each sub_buffer
	Vertex2D* vtx_dst = mappedVtxMemory;
	uint32_t* idx_dst = mappedIdxMemory;
	for (auto draw_list : draw_data.drawLists) 
	{
			memcpy(vtx_dst, draw_list.vertexBuffer,	
				draw_list.vertexBufferSize * sizeof(Vertex2D));
			memcpy(idx_dst, draw_list.indexBuffer, 
				draw_list.indexBufferSize * sizeof(uint32_t));
			vtx_dst += draw_list.vertexBufferSize;
			idx_dst += draw_list.indexBufferSize;
	}
	if (draw_data.totalIdxCount > 0) {
		VkMappedMemoryRange range[2] = {};
		range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[0].memory = vertexBufferMemory;
		range[0].size = VK_WHOLE_SIZE;
		range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[1].memory = indexBufferMemory;
		range[1].size = VK_WHOLE_SIZE;
		if (vkFlushMappedMemoryRanges(Device::get(), 2, range) != VK_SUCCESS) {
			std::runtime_error("Flush Memory ranges has failed");
		}
	}
	this->draw_data = draw_data;
}

UiDrawData GuiMesh::getData()
{
	return this->draw_data;
}

uint32_t GuiMesh::getIdxCount() const
{
	return this->draw_data.totalIdxCount;
}

GuiMesh::~GuiMesh()
{
	if (allocated_Idx_MemSize > 0) {
		vkUnmapMemory(Device::get(), indexBufferMemory);
		vkDestroyBuffer(Device::get(), indexBuffer, nullptr);
		vkFreeMemory(Device::get(), indexBufferMemory, nullptr);
	}
	if (allocated_Vtx_MemSize > 0) {
		vkUnmapMemory(Device::get(), vertexBufferMemory);
		vkDestroyBuffer(Device::get(), vertexBuffer, nullptr);
		vkFreeMemory(Device::get(), vertexBufferMemory, nullptr);
	}
}
