#pragma once
#include "commons.h"

class Shader
{
public:
	Shader(std::string spirv_Path , VkShaderStageFlagBits target_stage);
	VkShaderModule getModule();
	VkPipelineShaderStageCreateInfo getStage();
	~Shader();
private:
	void createShaderModule(const std::vector<char>& code);
	VkShaderModule module;
	VkShaderStageFlagBits target_stage;
};

