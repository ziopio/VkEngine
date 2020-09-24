#include "DescriptorSets.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "Pipeline.h"
#include "ApiUtils.h"
#include "TextureManager.h"
#include "Renderer.h"

std::vector<DescSetLayout> DescriptorSetsFactory::layouts;
std::vector<VkDescriptorPool> DescriptorSetsFactory::pools;
VkBuffer DescriptorSetsFactory::uniformBuffer;
VkDeviceMemory DescriptorSetsFactory::uniformBufferMemory;
void* DescriptorSetsFactory::mappedUniformMemory;

void DescriptorSetsFactory::initLayouts() {
	DescriptorSetsFactory::layouts.resize(DescSetsLayouts::DescSetsLayouts_END);

	// TEXTURE_ARRAY : 32 texture in fragment shader
	{
		DescriptorSetsFactory::layouts[TEXTURE_ARRAY].type = TEXTURE_ARRAY;
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = TEXTURE_ARRAY_LENGTH;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layouts[TEXTURE_ARRAY].bindings = { samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layouts[TEXTURE_ARRAY].bindings.size());
		layoutInfo.pBindings = layouts[TEXTURE_ARRAY].bindings.data();

		if (vkCreateDescriptorSetLayout(Device::get(), 
			&layoutInfo, nullptr, 
			&DescriptorSetsFactory::layouts[DescSetsLayouts::TEXTURE_ARRAY].layout) 
			!= VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	// FRAMEBUFFER_TEXTURE Texture : in fragment shader
	{
		DescriptorSetsFactory::layouts[FRAMEBUFFER_TEXTURE].type = FRAMEBUFFER_TEXTURE;
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layouts[FRAMEBUFFER_TEXTURE].bindings = { samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layouts[FRAMEBUFFER_TEXTURE].bindings.size());
		layoutInfo.pBindings = layouts[FRAMEBUFFER_TEXTURE].bindings.data();

		if (vkCreateDescriptorSetLayout(Device::get(),
			&layoutInfo, nullptr,
			&DescriptorSetsFactory::layouts[DescSetsLayouts::FRAMEBUFFER_TEXTURE].layout)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		layouts[FRAMEBUFFER_TEXTURE].frame_dependent = true;
	}
	// Uniform set for drawing in 3D
	{		
		DescriptorSetsFactory::layouts[UNIFORM_BUFFER].type = UNIFORM_BUFFER;
		VkDescriptorSetLayoutBinding uniformMatLayoutBinding = {};
		uniformMatLayoutBinding.binding = 0;
		uniformMatLayoutBinding.descriptorCount = 1;
		uniformMatLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformMatLayoutBinding.pImmutableSamplers = nullptr;
		uniformMatLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		layouts[UNIFORM_BUFFER].bindings = { uniformMatLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(layouts[UNIFORM_BUFFER].bindings.size());
		layoutInfo.pBindings = layouts[UNIFORM_BUFFER].bindings.data();

		if (vkCreateDescriptorSetLayout(Device::get(), 
			&layoutInfo, nullptr,
			&DescriptorSetsFactory::layouts[DescSetsLayouts::UNIFORM_BUFFER].layout)
			!= VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
		layouts[UNIFORM_BUFFER].frame_dependent = true;
	}
}

void DescriptorSetsFactory::initDescSetPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	auto frames_in_flight = static_cast<uint32_t>(SwapChainMng::get()->getImageViews().size());
	unsigned sets_needed = 0;

	for (auto & PLayout : PipelineFactory::pipeline_layouts) {
		// all static sets
		for (auto & DSLayout : PLayout.descriptors.static_sets) {
			for (auto & bind : DSLayout.layout->bindings) {
				VkDescriptorPoolSize poolSize;
				poolSize.type = bind.descriptorType;
				poolSize.descriptorCount = bind.descriptorCount;
					sets_needed++;
				poolSizes.push_back(poolSize);
			}
		}
		for (auto & DSLayouts : PLayout.descriptors.frame_dependent_sets) {
			for (auto & bind : DSLayouts[0].layout->bindings) {
				VkDescriptorPoolSize poolSize;
				poolSize.type = bind.descriptorType;
				poolSize.descriptorCount = bind.descriptorCount * frames_in_flight;
				sets_needed += frames_in_flight;
				poolSizes.push_back(poolSize);
			}
		}
	}

	//Second: select how many sets can be allocated
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	// 1 of uniforms per image + 1 for textures(3D scene) + 2 for IMGUI (textures and 3D scene render per swap image)
	poolInfo.maxSets = sets_needed;
	VkDescriptorPool pool;
	VkResult result = vkCreateDescriptorPool(Device::get(), &poolInfo, nullptr, &pool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
	DescriptorSetsFactory::pools.push_back(pool);
}

void DescriptorSetsFactory::allocateDescriptorSets(DescSetBundle * bundle)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	auto lambda = [&allocInfo](DescSet *desc_set) {
		allocInfo.descriptorPool = pools[0]; // ONE POOL FOR NOW
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &desc_set->layout->layout;
		VkResult result = vkAllocateDescriptorSets(Device::get(), &allocInfo, &desc_set->set);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	};

	for (auto & desc_set : bundle->static_sets) {
		lambda(&desc_set);
	}
	for (auto & set_list : bundle->frame_dependent_sets)
		for( auto & desc_set : set_list){
			lambda(&desc_set);
	}

}

void DescriptorSetsFactory::updateDescriptorSets(DescSetBundle* bundle)
{
	// this code aims to initialize the entire bundle with just one call to "vkUpdateDescriptorSets"
	std::vector<VkWriteDescriptorSet> writes;
	std::vector<std::vector<VkDescriptorImageInfo>> images_infos;
	std::vector<std::vector<VkDescriptorBufferInfo>> buffers_infos;
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

	for (auto & set : bundle->static_sets) {
		descriptorWrite.dstSet = set.set;
		for (auto & bind : set.layout->bindings) {
			descriptorWrite.dstBinding = bind.binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = bind.descriptorType;
			descriptorWrite.descriptorCount = bind.descriptorCount;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = nullptr;
			switch (bind.descriptorType) {
			case VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				images_infos.push_back(gatherImageInfos( set.purpose , bundle->data_context));
				descriptorWrite.pImageInfo = images_infos.back().data();
				break;
			default: 
				std::runtime_error("Tried to initialize a Descriptor set NOT supported by the engine!");
			}
			writes.push_back(descriptorWrite);
		}
	}
	for (auto & set_group : bundle->frame_dependent_sets) {
		for (int i = 0; i < set_group.size(); i++) {
			descriptorWrite.dstSet = set_group[i].set;
			for (auto & bind : set_group[i].layout->bindings) {
				descriptorWrite.dstBinding = bind.binding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = bind.descriptorType;
				descriptorWrite.descriptorCount = bind.descriptorCount;
				descriptorWrite.pBufferInfo = nullptr;
				descriptorWrite.pImageInfo = nullptr;
				switch (bind.descriptorType) 
				{
				case VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
					VkDescriptorBufferInfo buff_info = {};
					buff_info.buffer = uniformBuffer;
					buff_info.range = sizeof(UniformBlock);
					VkDeviceSize minAlignement =
						PhysicalDevice::getPhysicalDeviceProperties().properties.limits.minUniformBufferOffsetAlignment;
					VkDeviceSize alignemetPadding = minAlignement - (sizeof(UniformBlock) % minAlignement);
					buff_info.offset = i * (sizeof(UniformBlock) + alignemetPadding);
					buffers_infos.push_back({ buff_info });
					descriptorWrite.pBufferInfo = buffers_infos.back().data();
				}break;
				case VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = Renderer::getOffScreenFrameAttachment(i).imageView;
					imageInfo.sampler = Renderer::getOffScreenFrameAttachment(i).Sampler;
					images_infos.push_back({ imageInfo });
					descriptorWrite.pImageInfo = images_infos.back().data();
				}
					break;
				default:
					std::runtime_error("Tried to initialize a Descriptor set NOT supported by the engine!");
				}
				writes.push_back(descriptorWrite);
			}
		}
	}
	vkQueueWaitIdle(Device::getGraphicQueue()); // wait for queue to be free
	vkUpdateDescriptorSets(Device::get(), writes.size(), writes.data(), 0, nullptr);
}

void DescriptorSetsFactory::updateUniformBuffer(UniformBlock uniforms, int imageIndex)
{
	VkDeviceSize minAlignement =
		PhysicalDevice::getPhysicalDeviceProperties().properties.limits.minUniformBufferOffsetAlignment;
	VkDeviceSize alignemetPadding = minAlignement - (sizeof(UniformBlock) % minAlignement);

	memcpy((char *)mappedUniformMemory + imageIndex * (sizeof(UniformBlock) + alignemetPadding),
		&uniforms, sizeof(uniforms));
}

void DescriptorSetsFactory::cleanUp() {
	vkUnmapMemory(Device::get(), uniformBufferMemory);
	vkDestroyBuffer(Device::get(), uniformBuffer, nullptr);
	vkFreeMemory(Device::get(), uniformBufferMemory, nullptr);
	for (auto & pool : pools) {
		vkDestroyDescriptorPool(Device::get(), pool, nullptr);
	}
	for (auto & DSlayout : layouts) {
		vkDestroyDescriptorSetLayout(Device::get(), DSlayout.layout, nullptr);
	}
	pools.clear();
	layouts.clear();
}

void DescriptorSetsFactory::createUniformBuffer()
{
	// one uniform block for each frame in-flight
	VkDeviceSize minAlignement =
		PhysicalDevice::getPhysicalDeviceProperties().properties.limits.minUniformBufferOffsetAlignment;
	VkDeviceSize alignemetPadding = minAlignement - (sizeof(UniformBlock) % minAlignement);

	VkDeviceSize bufferSize = (sizeof(UniformBlock) + alignemetPadding) * SwapChainMng::get()->getImageCount();

	createBuffer(PhysicalDevice::get(), Device::get(), bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		uniformBuffer, uniformBufferMemory);
	vkMapMemory(Device::get(),
		uniformBufferMemory,
		0, // offset in bytes 
		bufferSize, //range to be mapped
		0, // flags
		&mappedUniformMemory); // Pointer location
}

std::vector<VkDescriptorImageInfo> DescriptorSetsFactory::gatherImageInfos(DescSetUsage usage, DescSetsResourceContext data_context)
{
	std::vector<VkDescriptorImageInfo> imagesInfo;

	switch (data_context)
	{
	case DescSetsResourceContext::SCENE_DATA:
		switch (usage)
		{
		case DescSetUsage::ALBEDO_TEXTURE:
			imagesInfo.resize(TEXTURE_ARRAY_LENGTH);
			for (int i = 0; i < TEXTURE_ARRAY_LENGTH; i++) {
				imagesInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (i < TextureManager::countSceneTextures()) {
					imagesInfo[i].imageView = TextureManager::getSceneTexture(i)->getTextureImgView();
					imagesInfo[i].sampler = TextureManager::getSceneTexture(i)->getTextureSampler();
				}
				else {
					imagesInfo[i].imageView = TextureManager::getSceneTexture(0)->getTextureImgView();
					imagesInfo[i].sampler = TextureManager::getSceneTexture(0)->getTextureSampler();
				}
			}
			break;
		}
		break;
	case DescSetsResourceContext::IMGUI_DATA:
		switch (usage)
		{
		case DescSetUsage::ALBEDO_TEXTURE:
			imagesInfo.resize(TEXTURE_ARRAY_LENGTH);
			for (int i = 0; i < TEXTURE_ARRAY_LENGTH; i++) {
				imagesInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				if (i < TextureManager::countImGuiTextures()) {
					imagesInfo[i].imageView = TextureManager::getImGuiTexture(i)->getTextureImgView();
					imagesInfo[i].sampler = TextureManager::getImGuiTexture(i)->getTextureSampler();
				}
				else {
					imagesInfo[i].imageView = TextureManager::getImGuiTexture(0)->getTextureImgView();
					imagesInfo[i].sampler = TextureManager::getImGuiTexture(0)->getTextureSampler();
				}
			}
			break;
		}
		break;
	default: std::runtime_error("Tried to use data context not managed: " + data_context);
		break;
	}
	return imagesInfo;
}


