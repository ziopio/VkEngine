#pragma once
#include "stdafx.h"

class Shader
{
public:
	Shader(std::string spirv_Path);
	VkShaderModule get();
	~Shader();
private:
	void createShaderModule(const std::vector<char>& code);
	VkShaderModule module;
};

