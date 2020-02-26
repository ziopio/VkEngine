#pragma once
#include "Material.h"

constexpr auto MAX_TEXTURE_COUNT = 32;
using namespace vkengine;

struct uniformBlockDefinition {
	glm::mat4 P_matrix;
	glm::mat4 V_matrix;
	LightData lights[10];
	uint32_t light_count;
};

struct PushConstantBlock {
	glm::mat4 model_transform;
	uint32_t textureIndex;
};

struct ImGuiPushConstantBlock {
	glm::vec2 uScale;
	glm::vec2 uTranslate;
	uint32_t tex_ID;
};

class MaterialManager
{
public:
	static void init(SwapChain* swapchain, RenderPass* renderer);
	static Material* getMaterial(MaterialType material);
	static VkDescriptorSetLayout getImGuiTextureArrayDescSetLayout();
	static VkDescriptorSetLayout getOffScreenTextureDescSetLayout();

	static VkDescriptorSetLayout getTextureDescriptorSetLayout();
	static VkDescriptorSetLayout getFrameDependentDescriptorSetLayout();
	static void destroyAllMaterials();
private:
	static void loadMaterials();
	static void createDescriptorSetLayouts();
	static VkDescriptorSetLayout imGuiTextureArrayDescSetLayout;
	static VkDescriptorSetLayout offScreenTextureDescSetLayout;
	static VkDescriptorSetLayout globalTextureDescriptorSetLayout;
	static VkDescriptorSetLayout frameDependentDescriptorSetLayout;
	static std::vector<Material*> materials;
	static SwapChain* swapchain;
	static RenderPass* renderPass;
	static bool ready;
};

