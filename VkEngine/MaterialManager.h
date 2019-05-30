#pragma once
#include "Material.h"

struct Light {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 power;
};

struct uniBlock {
	glm::mat4 P_matrix;
	glm::mat4 V_matrix;
	Light lights[10];
	int light_count;
};

struct PushConstantBlock {
	glm::mat4 model_transform;
	int textureIndex;
};

class MaterialManager
{
public:
	static void init(SwapChain* swapchain, RenderPass* renderer);
	static Material* getMaterial(MaterialType material);
	static void destroyAllMaterials();
private:
	static void loadMaterials();
	static std::vector<Material*> materials;
	static SwapChain* swapchain;
	static RenderPass* renderPass;
	static bool ready;
};

