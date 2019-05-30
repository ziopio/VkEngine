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
	VkDescriptorSetLayout getDescriptorSetLayout();
	~Material();
private:
	void createDescriptorSetLayout();
	void buildPipeline();
	SwapChain* swapChain;
	RenderPass* renderPass;
	MaterialType type;
	Shader* vertexShader;
	Shader* fragmentShader;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
};

