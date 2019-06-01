#include "stdafx.h"
#include "DescriptorSetsFactory.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "ApiUtils.h"

static const uniBlock uniform_sample;

SwapChain* DescriptorSetsFactory::swapChain;
std::vector<Object*> DescriptorSetsFactory::objects;
std::multimap<MaterialType, Object*> DescriptorSetsFactory::material2obj_map;
VkDescriptorPool DescriptorSetsFactory::descriptorPool;
std::vector<VkDescriptorSet> DescriptorSetsFactory::descriptorSets;
std::vector<void *>DescriptorSetsFactory::mappedUniformMemory;
std::vector<VkBuffer> DescriptorSetsFactory::uniformBuffers;
std::vector<VkDeviceMemory> DescriptorSetsFactory::uniformBuffersMemory;
bool DescriptorSetsFactory::ready;

void DescriptorSetsFactory::init(SwapChain* swapChain, std::vector<Object*> objects)
{
	DescriptorSetsFactory::swapChain = swapChain;
	DescriptorSetsFactory::objects = objects;
	//mapMaterialsToObjects();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	ready = true;
}

VkDescriptorSet* DescriptorSetsFactory::getDescriptorSet(MaterialType material)
{
	if (!ready) {
		throw std::runtime_error("DescriptorSetsFactory not ready");
	}
	return &descriptorSets[material];
}

void DescriptorSetsFactory::updateUniformBuffer(uniBlock uniforms, int imageIndex)
{
	memcpy(mappedUniformMemory[imageIndex], &uniforms, sizeof(uniforms));
}

void DescriptorSetsFactory::cleanUp()
{
	for (int i = 0; i < swapChain->getImageViews().size();i++) {
		vkUnmapMemory(Device::get(), uniformBuffersMemory[i]);
		vkDestroyBuffer(Device::get(), uniformBuffers[i], nullptr);
		vkFreeMemory(Device::get(), uniformBuffersMemory[i], nullptr);
	}
	vkDestroyDescriptorPool(Device::get(), descriptorPool, nullptr);
}

void DescriptorSetsFactory::mapMaterialsToObjects()
{
	for (auto obj : objects) {
		material2obj_map.insert(std::pair<MaterialType, Object*>(obj->getMatType(),obj));
	}
}

void DescriptorSetsFactory::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts;
	for ( int i = 0; i < MaterialType::LAST; i++) {
		layouts.push_back(MaterialManager::getMaterial((MaterialType)i)->getDescriptorSetLayouts());
	}
	
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 2;//static_cast<uint32_t>(swapChain->getImageViews().size());
	allocInfo.pSetLayouts = layouts.data(); // same size of descriptorSet array

	descriptorSets.resize(MaterialType::LAST);
	VkResult result = vkAllocateDescriptorSets(Device::get(), &allocInfo, descriptorSets.data());
	if( result != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	//L'array di textures sarà inizializzato allo stesso modo per tutti i materiali
	std::array<VkDescriptorImageInfo, MAX_TEXTURE_COUNT>  imagesInfo = {};
	for (int i = 0; i < MAX_TEXTURE_COUNT; i++) {
		imagesInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (i < objects.size()) {
			imagesInfo[i].imageView = TextureManager::getTexture(objects[i]->getTextureId())->getTextureImgView();
			imagesInfo[i].sampler = TextureManager::getTexture(objects[i]->getTextureId())->getTextureSampler();
		} else {
			imagesInfo[i].imageView = TextureManager::getTexture(0)->getTextureImgView();
			imagesInfo[i].sampler = TextureManager::getTexture(0)->getTextureSampler();
		}
	}
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(uniform_sample);

	// Tutti i materiali supportano lo stesso array di texture
	// 1 descriptor set per ogni materiale
	for (int i = 0; i < MaterialType::LAST ;i++) {
		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = imagesInfo.size();
		descriptorWrites[0].pImageInfo = imagesInfo.data();

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(Device::get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void DescriptorSetsFactory::createDescriptorPool()
{
	// First: select which types of descriptors the pool handles and how many descriptors per type
	// I need 1 descriptor for each type, for each swapchain image
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	// here i need 1 uniform descriptor for each frame in flight
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChain->getImageViews().size());
	// here i need as many samplers as the size of the texture2D array in the shader
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_TEXTURE_COUNT;

	//Second: select how many sets can be allocated
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(swapChain->getImageViews().size()) + 1;

	VkResult result = vkCreateDescriptorPool(Device::get(), &poolInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void DescriptorSetsFactory::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(uniform_sample);

	uniformBuffers.resize(swapChain->getImageViews().size());
	uniformBuffersMemory.resize(swapChain->getImageViews().size());

	for (size_t i = 0; i < swapChain->getImageViews().size(); i++) {
		createBuffer(PhysicalDevice::get(), Device::get(), bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i], uniformBuffersMemory[i]);

		vkMapMemory(Device::get(), uniformBuffersMemory[i], 0, sizeof(uniform_sample), 0, &mappedUniformMemory[i]);
	}
}
