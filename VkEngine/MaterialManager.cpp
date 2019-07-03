#include "stdafx.h"
#include "Device.h"
#include "MaterialManager.h"
#include "RenderPass.h"


std::vector<Material*> MaterialManager::materials;
SwapChain*  MaterialManager::swapchain;
RenderPass* MaterialManager::renderPass;
bool MaterialManager::ready; 
VkDescriptorSetLayout MaterialManager::imGuiTextureArrayDescSetLayout;
VkDescriptorSetLayout MaterialManager::offScreenTextureDescSetLayout;

VkDescriptorSetLayout MaterialManager::globalTextureDescriptorSetLayout;
VkDescriptorSetLayout MaterialManager::frameDependentDescriptorSetLayout;


void MaterialManager::init(SwapChain * swapchain, RenderPass * renderPass)
{
	MaterialManager::swapchain = swapchain;
	MaterialManager::renderPass = renderPass;
	createDescriptorSetLayouts();
	loadMaterials();
	ready = true;
}

Material* MaterialManager::getMaterial(MaterialType material)
{
	if (!ready) {
		throw std::runtime_error("Material Manager not initialized!");
	}
	return MaterialManager::materials[material];
}

VkDescriptorSetLayout MaterialManager::getImGuiTextureArrayDescSetLayout()
{
	return imGuiTextureArrayDescSetLayout;
}

VkDescriptorSetLayout MaterialManager::getOffScreenTextureDescSetLayout()
{
	return offScreenTextureDescSetLayout;
}

VkDescriptorSetLayout MaterialManager::getTextureDescriptorSetLayout()
{
	return MaterialManager::globalTextureDescriptorSetLayout;
}

VkDescriptorSetLayout MaterialManager::getFrameDependentDescriptorSetLayout()
{
	return MaterialManager::frameDependentDescriptorSetLayout;
}


void MaterialManager::destroyAllMaterials()
{
	for (auto mat : MaterialManager::materials) {
		delete mat;
	}
	MaterialManager::materials.clear();	
	vkDestroyDescriptorSetLayout(Device::get(), imGuiTextureArrayDescSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(Device::get(), offScreenTextureDescSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(Device::get(), globalTextureDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(Device::get(), frameDependentDescriptorSetLayout, nullptr);
	ready = false;
}



void MaterialManager::loadMaterials()
{
	MaterialManager::materials.push_back(new Material(MaterialType::SAMPLE, swapchain, renderPass));

	MaterialManager::materials.push_back(new Material(MaterialType::PHONG, swapchain, renderPass));

	MaterialManager::materials.push_back(new Material(MaterialType::UI, swapchain, renderPass));

}


void MaterialManager::createDescriptorSetLayouts()
{
	// There will be 2 sets + 1 stand alone set for ImGui
	// First set
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = MAX_TEXTURE_COUNT;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		std::array<VkDescriptorSetLayoutBinding, 1> set_0_bindings = { samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(set_0_bindings.size());
		layoutInfo.pBindings = set_0_bindings.data();

		if (vkCreateDescriptorSetLayout(Device::get(), &layoutInfo, nullptr, &globalTextureDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
	// Second set
	{
		VkDescriptorSetLayoutBinding uniformMatLayoutBinding = {};
		uniformMatLayoutBinding.binding = 0;
		uniformMatLayoutBinding.descriptorCount = 1;
		uniformMatLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformMatLayoutBinding.pImmutableSamplers = nullptr;
		uniformMatLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		std::array<VkDescriptorSetLayoutBinding, 1> set_1_bindings = { uniformMatLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(set_1_bindings.size());
		layoutInfo.pBindings = set_1_bindings.data();

		if (vkCreateDescriptorSetLayout(Device::get(), &layoutInfo, nullptr, &frameDependentDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
	// ImGui set for gui textures and subPassInput
	{
		// binding 0 has a MAX_TEXTURE_COUNT texture array
		VkDescriptorSetLayoutBinding samplerArrayLayoutBinding = {};
		samplerArrayLayoutBinding.binding = 0;
		samplerArrayLayoutBinding.descriptorCount = MAX_TEXTURE_COUNT;
		samplerArrayLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerArrayLayoutBinding.pImmutableSamplers = nullptr;
		samplerArrayLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> set_0_bindings = { samplerArrayLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(set_0_bindings.size());
		layoutInfo.pBindings = set_0_bindings.data();

		if (vkCreateDescriptorSetLayout(Device::get(), &layoutInfo, nullptr, &imGuiTextureArrayDescSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
	{
		VkDescriptorSetLayoutBinding singleSamplerLayoutBinding = {};
		singleSamplerLayoutBinding.binding = 0;
		singleSamplerLayoutBinding.descriptorCount = 1;
		singleSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		singleSamplerLayoutBinding.pImmutableSamplers = nullptr;
		singleSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// binding 1 represents the input attachment of a previus renderPass
		VkDescriptorSetLayoutBinding dynamicTextureLayoutBinding = {};
		dynamicTextureLayoutBinding.binding = 0;
		dynamicTextureLayoutBinding.descriptorCount = 1;
		dynamicTextureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dynamicTextureLayoutBinding.pImmutableSamplers = nullptr;
		dynamicTextureLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		std::array<VkDescriptorSetLayoutBinding, 1> set_1_bindings = { dynamicTextureLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(set_1_bindings.size());
		layoutInfo.pBindings = set_1_bindings.data();

		if (vkCreateDescriptorSetLayout(Device::get(), &layoutInfo, nullptr, &offScreenTextureDescSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

}
