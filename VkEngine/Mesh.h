#pragma once
#include "VkEngine.h"

using namespace vkengine;

struct Vertex3D {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};
		attributeDescriptions.resize(4);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 format
		attributeDescriptions[0].offset = offsetof(Vertex3D, pos); //the position of pos bits inside the Vertex struct 
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, normal);
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3D, color);
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex3D, texCoord);
		return attributeDescriptions;
	}
	bool operator==(const Vertex3D& other) const {
		return pos == other.pos && 
			normal == other.normal &&
			color == other.color && 
			texCoord == other.texCoord;
	}
};

struct Vertex2D {
	glm::vec2 pos;
	glm::vec2 texCoord;
	uint32_t color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex2D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};
		attributeDescriptions.resize(3);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // vec2 format
		attributeDescriptions[0].offset = offsetof(Vertex2D, pos); //the position of pos bits inside the Vertex struct 
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT; 
		attributeDescriptions[1].offset = offsetof(Vertex2D, texCoord);
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM; //uint32 to vec4
		attributeDescriptions[2].offset = offsetof(Vertex2D, color);
		return attributeDescriptions;
	}
	bool operator==(const Vertex2D& other) const {
		return pos == other.pos &&
			color == other.color &&
			texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex3D> {
		size_t operator()(Vertex3D const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

class BaseMesh {
public:
	virtual VkBuffer getVkVertexBuffer();
	virtual VkBuffer getVkIndexBuffer();
	virtual uint32_t getIdxCount() = 0;
protected:
	VkDeviceMemory vertexBufferMemory;
	VkBuffer vertexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkBuffer indexBuffer;
};

class Mesh : public BaseMesh
{
public:
	Mesh(std::string modelPath);
	uint32_t getIdxCount() override;
	~Mesh();
private:
	void loadModel(std::string modelPath);
	void createVertexBuffer();
	void createIndexBuffer();
	std::vector<Vertex3D> vertices;
	std::vector<uint32_t> indices;
};

class GuiMesh : public BaseMesh {
public:
	GuiMesh();
	//Update buffers with new fresh data, 
	//if space is not enough, buffers and memory 
	//are re-allocated with the double of needed space.
	void updateMeshData(UiDrawData draw_data);
	UiDrawData getData();
	uint32_t getIdxCount() override;
	~GuiMesh();
private:
	UiDrawData draw_data;
	Vertex2D* mappedVtxMemory;
	uint32_t* mappedIdxMemory;
	size_t allocated_Vtx_MemSize;
	size_t allocated_Idx_MemSize;
};
