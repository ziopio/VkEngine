#pragma once

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
	glm::vec4 color;

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
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT; //vec4
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

class Mesh
{
public:
	Mesh(std::string modelPath);
	VkBuffer getVkVertexBuffer();
	VkBuffer getVkIndexBuffer();
	~Mesh();
	std::vector<Vertex3D> vertices;
	std::vector<uint32_t> indices;
private:
	void loadModel(std::string modelPath);
	void createVertexBuffer();
	void createIndexBuffer();
	VkDeviceMemory vertexBufferMemory;
	VkBuffer vertexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkBuffer indexBuffer;
};

