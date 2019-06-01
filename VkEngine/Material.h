#pragma once
#include "Shader.h"
#include "RenderPass.h"

enum MaterialType {
	SAMPLE,
	PHONG,
	LAST // DUMMY TOKEN leave at END!
};


class Material
{
public:
	Material(MaterialType material, SwapChain* swapchain, RenderPass* renderer);
	VkPipeline getPipeline();
	VkPipelineLayout getPipelineLayout();
	std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts();
	~Material();
private:
	void createDescriptorSetLayouts();
	void buildPipeline();
	SwapChain* swapChain;
	RenderPass* renderPass;
	MaterialType type;
	Shader* vertexShader;
	Shader* fragmentShader;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
};

