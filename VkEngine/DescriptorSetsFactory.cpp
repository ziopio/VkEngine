#include "stdafx.h"
#include "DescriptorSetsFactory.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "ApiUtils.h"

SwapChain* DescriptorSetsFactory::swapChain;
std::multimap<MaterialType, Object*> DescriptorSetsFactory::material2obj_map;
VkDescriptorPool DescriptorSetsFactory::descriptorPool;
VkDescriptorSet DescriptorSetsFactory::imGuiDescriptorSet;
VkDescriptorSet DescriptorSetsFactory::staticGlobalDescriptorSet;
std::vector<VkDescriptorSet> DescriptorSetsFactory::frameDescriptorSets;
std::vector<VkDescriptorSet> DescriptorSetsFactory::materialDescriptorSets;
void* DescriptorSetsFactory::mappedUniformMemory;
VkBuffer DescriptorSetsFactory::uniformBuffers;
VkDeviceMemory DescriptorSetsFactory::uniformBuffersMemory;
bool DescriptorSetsFactory::ready;

void DescriptorSetsFactory::init(SwapChain* swapChain)
{
	DescriptorSetsFactory::swapChain = swapChain;
	createFrameDependentUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	ready = true;
}
void DescriptorSetsFactory::updateUniformBuffer(uniformBlockDefinition uniforms, int imageIndex)
{
	VkDeviceSize minAlignement =
		PhysicalDevice::getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	VkDeviceSize alignemetPadding = minAlignement - (sizeof(uniformBlockDefinition) % minAlignement);

	memcpy((char *)mappedUniformMemory + imageIndex * (sizeof(uniformBlockDefinition) + alignemetPadding ),
		&uniforms, sizeof(uniforms));
}

VkDescriptorSet DescriptorSetsFactory::getFrameDescriptorSet(int frame_index)
{
	return frameDescriptorSets[frame_index];
}

VkDescriptorSet DescriptorSetsFactory::getMaterialDescriptorSets()
{
	throw std::runtime_error("not implemented");
	return VkDescriptorSet();
}

VkDescriptorSet DescriptorSetsFactory::getStaticGlobalDescriptorSet()
{
	return staticGlobalDescriptorSet;
}

VkDescriptorSet DescriptorSetsFactory::getImGuiDescriptorSet()
{
	return imGuiDescriptorSet;
}

void DescriptorSetsFactory::cleanUp()
{
	vkUnmapMemory(Device::get(), uniformBuffersMemory);
	vkDestroyBuffer(Device::get(), uniformBuffers, nullptr);
	vkFreeMemory(Device::get(), uniformBuffersMemory, nullptr);
	vkDestroyDescriptorPool(Device::get(), descriptorPool, nullptr);
}

void DescriptorSetsFactory::createDescriptorSets()
{
	//Static uniforms descriptor sets allocation and definition
	{
		auto layout = MaterialManager::getTextureDescriptorSetLayout();
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout; // same size of descriptorSet array

		VkResult result = vkAllocateDescriptorSets(Device::get(), &allocInfo, &staticGlobalDescriptorSet);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		//L'array di textures sar� inizializzato allo stesso modo per tutti i materiali
		std::array<VkDescriptorImageInfo, MAX_TEXTURE_COUNT>  imagesInfo = {};
		for (int i = 0; i < MAX_TEXTURE_COUNT; i++) {
			imagesInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (i < TextureManager::getTextureCount()) {
				imagesInfo[i].imageView = TextureManager::getTexture(i)->getTextureImgView();
				imagesInfo[i].sampler = TextureManager::getTexture(i)->getTextureSampler();
			}
			else {
				imagesInfo[i].imageView = TextureManager::getTexture(0)->getTextureImgView();
				imagesInfo[i].sampler = TextureManager::getTexture(0)->getTextureSampler();
			}
		}
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = staticGlobalDescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = imagesInfo.size();
		descriptorWrite.pImageInfo = imagesInfo.data();

		vkUpdateDescriptorSets(Device::get(), 1, &descriptorWrite, 0, nullptr);
	}
	// Uniform Descriptor Set for data that changes one time each frame
	{
		frameDescriptorSets.resize(swapChain->getImageViews().size());
		std::vector<VkDescriptorSetLayout> frame_dependent_layouts;
		frame_dependent_layouts.resize(swapChain->getImageViews().size(), MaterialManager::getFrameDependentDescriptorSetLayout());
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = frame_dependent_layouts.size();
		allocInfo.pSetLayouts = frame_dependent_layouts.data(); // same size of descriptorSet array

		VkResult result = vkAllocateDescriptorSets(Device::get(), &allocInfo, frameDescriptorSets.data());
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		VkDeviceSize minAlignement =
			PhysicalDevice::getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
		VkDeviceSize alignemetPadding = minAlignement - (sizeof(uniformBlockDefinition) % minAlignement);

		std::vector<VkDescriptorBufferInfo> bufferInfos = {};
		bufferInfos.resize(frameDescriptorSets.size());

		std::vector<VkWriteDescriptorSet> descriptorWrites = {};
		descriptorWrites.resize(frame_dependent_layouts.size());

		for (int i = 0; i < descriptorWrites.size(); i++) {
			//Each descriptor set points to the same uniform buffer but with a different offset
			bufferInfos[i].buffer = DescriptorSetsFactory::uniformBuffers;
			bufferInfos[i].range = sizeof(uniformBlockDefinition); 
			// offset in bytes in the uniform buffer
			bufferInfos[i].offset = i * (sizeof(uniformBlockDefinition) + alignemetPadding);

			descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[i].dstSet = frameDescriptorSets[i];
			descriptorWrites[i].dstBinding = 0;
			descriptorWrites[i].dstArrayElement = 0;
			descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[i].descriptorCount = 1;
			descriptorWrites[i].pBufferInfo = &bufferInfos[i];
		}
		vkUpdateDescriptorSets(Device::get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	//Static set for imgui texture samplers
	{
		auto layout = MaterialManager::getImGuiDescriptorSetLayout();
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout; // same size of descriptorSet array

		VkResult result = vkAllocateDescriptorSets(Device::get(), &allocInfo, &imGuiDescriptorSet);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		//temporaneamente tutti i descrittori punteranno allo stesso sampler
		std::array<VkDescriptorImageInfo, MAX_TEXTURE_COUNT>  imagesInfo = {};
		for (int i = 0; i < MAX_TEXTURE_COUNT; i++) {
			imagesInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imagesInfo[i].imageView = TextureManager::getFontAtlasTexture()->getTextureImgView();
			imagesInfo[i].sampler = TextureManager::getFontAtlasTexture()->getTextureSampler();
		}
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = imGuiDescriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = imagesInfo.size();
		descriptorWrite.pImageInfo = imagesInfo.data();

		vkUpdateDescriptorSets(Device::get(), 1, &descriptorWrite, 0, nullptr);
	}
}

void DescriptorSetsFactory::createDescriptorPool()
{
	// First: select which types of descriptors the pool handles and how many descriptors per type
	// I need 1 descriptor for each type, for each swapchain image
	std::array<VkDescriptorPoolSize, 3> poolSizes = {};
	// here i need 1 uniform descriptor for each frame in flight
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChain->getImageViews().size());
	// here i need as many samplers as the size of the texture2D array in the shader
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_TEXTURE_COUNT;
	// same samplers for another array used in ImGui shaders
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = MAX_TEXTURE_COUNT;

	//Second: select how many sets can be allocated
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	// 1 per image + 1 for textures(3D scene) + 1 for IMGUI (2D gui)
	poolInfo.maxSets = static_cast<uint32_t>(swapChain->getImageViews().size()) + 1 + 1;

	VkResult result = vkCreateDescriptorPool(Device::get(), &poolInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void DescriptorSetsFactory::createFrameDependentUniformBuffers()
{
	// one uniform block for each frame in-flight
	VkDeviceSize minAlignement =
		PhysicalDevice::getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	VkDeviceSize alignemetPadding = minAlignement - (sizeof(uniformBlockDefinition) % minAlignement);

	VkDeviceSize bufferSize = (sizeof(uniformBlockDefinition) + alignemetPadding ) * swapChain->getImageViews().size();

	createBuffer(PhysicalDevice::get(), Device::get(), bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers, uniformBuffersMemory);
	vkMapMemory(Device::get(), 
		uniformBuffersMemory,
		0, // offset in bytes 
		bufferSize, //range to be mapped
		0, // flags
		&mappedUniformMemory); // Pointer location
}
