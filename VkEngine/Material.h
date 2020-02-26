#pragma once
#include "Shader.h"
#include "RenderPass.h"

using namespace vkengine;

class Material
{
public:
	Material(MaterialType material, SwapChain* swapchain, RenderPass* renderer);
	VkPipeline getPipeline();
	VkPipelineLayout getPipelineLayout();
	static VkDescriptorSetLayout getMaterialSpecificDescriptorSetLayout();

	~Material();
private:
	void createMaterialSpecificDescriptorSetLayouts();
	void buildPipeline();
	SwapChain* swapChain;
	RenderPass* renderPass;
	MaterialType type;
	Shader* vertexShader;
	Shader* fragmentShader;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout materialSpecificDescriptorSetLayout;
};

