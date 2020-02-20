#include "stdafx.h"
#include "Shader.h"
#include "Device.h"


static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary); // leggi dal basso e salva i dati in binario

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + filename);
	}
	// la dimensione in byte è equivalente alla posizione di lettura
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

Shader::Shader(std::string spirv_Path)
{
	auto code = readFile(spirv_Path);
	createShaderModule(code);
}

VkShaderModule Shader::get()
{
	return this->module;
}

Shader::~Shader()
{
	vkDestroyShaderModule(Device::get(), this->module, nullptr);
}

void Shader::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(Device::get(), &createInfo, nullptr, &module) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
}
