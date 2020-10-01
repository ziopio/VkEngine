#include "commons.h"
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

Shader::Shader(std::string spirv_Path, VkShaderStageFlagBits target_stage)
{
	auto code = readFile(spirv_Path);
	createShaderModule(code);
	this->target_stage = target_stage;
}

VkShaderModule Shader::getModule()
{
	return this->module;
}

VkPipelineShaderStageCreateInfo Shader::getStage()
{
	VkPipelineShaderStageCreateInfo info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	info.stage = this->target_stage;
	info.module = this->module;
	info.pName = "main"; // Si può personalizzare l'entry-point
	return info;
}

Shader::~Shader()
{
	if (this->module != VK_NULL_HANDLE) {
		vkDestroyShaderModule(Device::get(), this->module, nullptr);
	}
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
