#include "DescriptorSets.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "Pipeline.h"
#include "ApiUtils.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "Renderer.h"

std::vector<DescSetLayout> DescriptorSetsFactory::layouts;
VkDescriptorPool DescriptorSetsFactory::pool;
VkBuffer DescriptorSetsFactory::uniformBuffer;
VkDeviceMemory DescriptorSetsFactory::uniformBufferMemory;
void* DescriptorSetsFactory::mappedUniformMemory;

VkDescriptorSetLayout createDStLayout(std::vector<VkDescriptorSetLayoutBinding> bindings) {
	VkDescriptorSetLayout layout;
	VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	if (vkCreateDescriptorSetLayout(Device::get(), &layoutInfo, nullptr,&layout) != VK_SUCCESS)
	{ throw std::runtime_error("failed to create descriptor set layout!"); }
	return layout;
}

void DescriptorSetsFactory::initLayouts() {
	DescriptorSetsFactory::layouts.resize(DescSetsLayouts::DescSetsLayouts_END);

	// RT STATIC DESC_SET : 1 binding of 1 ACs in raytracing shaders
	{

		VkDescriptorSetLayoutBinding vertexStorageBinding = {};
		vertexStorageBinding.binding = 0;
		vertexStorageBinding.descriptorCount = SUPPORTED_MESH_COUNT; // TODO make dynamic
		vertexStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vertexStorageBinding.stageFlags = VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		VkDescriptorSetLayoutBinding indexStorageBinding = {};
		indexStorageBinding.binding = 1;
		indexStorageBinding.descriptorCount = SUPPORTED_MESH_COUNT; // TODO  make dynamic
		indexStorageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		indexStorageBinding.stageFlags = VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		VkDescriptorSetLayoutBinding samplerArrayBinding = {};
		samplerArrayBinding.binding = 2;
		samplerArrayBinding.descriptorCount = SUPPORTED_TEXTURE_COUNT; // TODO  make dynamic
		samplerArrayBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerArrayBinding.stageFlags = VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		layouts[DSL_RAY_TRACING_SCENE].bindings = { vertexStorageBinding, indexStorageBinding, samplerArrayBinding };
		layouts[DSL_RAY_TRACING_SCENE].layout = createDStLayout(layouts[DSL_RAY_TRACING_SCENE].bindings);
	}
	// TEXTURE_ARRAY : 1 binding of 32 textures in fragment shader
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = SUPPORTED_TEXTURE_COUNT;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layouts[DSL_TEXTURE_ARRAY].bindings = { samplerLayoutBinding };
		layouts[DSL_TEXTURE_ARRAY].layout = createDStLayout(layouts[DSL_TEXTURE_ARRAY].bindings);
	}
	// STORAGE_IMAGE for RAYTRACING : 1 binding of 1 storage image and 1 sceneObj buffer in raytracing shaders
	{
		VkDescriptorSetLayoutBinding accStructBinding = {};
		accStructBinding.binding = 0;
		accStructBinding.descriptorCount = 1;
		accStructBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		accStructBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		VkDescriptorSetLayoutBinding storageImgLayoutBinding = {};
		storageImgLayoutBinding.binding = 1;
		storageImgLayoutBinding.descriptorCount = 1;
		storageImgLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		storageImgLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		VkDescriptorSetLayoutBinding sceneDescBinding = {};
		sceneDescBinding.binding = 2;
		sceneDescBinding.descriptorCount = 1;
		sceneDescBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		sceneDescBinding.stageFlags = VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		layouts[DSL_RT_IMAGE_AND_OBJECTS].bindings = { accStructBinding, storageImgLayoutBinding, sceneDescBinding };
		layouts[DSL_RT_IMAGE_AND_OBJECTS].layout = createDStLayout(layouts[DSL_RT_IMAGE_AND_OBJECTS].bindings);
	}
	// FRAMEBUFFER_TEXTURE: a set with 1binding of 1 Texture : in fragment shader
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layouts[DSL_FRAMEBUFFER_TEXTURE].bindings = { samplerLayoutBinding };
		layouts[DSL_FRAMEBUFFER_TEXTURE].layout = createDStLayout(layouts[DSL_FRAMEBUFFER_TEXTURE].bindings);
	}
	// Uniform set for drawing both in rasterization and in raytracing
	{		
		VkDescriptorSetLayoutBinding uniformMatLayoutBinding = {};
		uniformMatLayoutBinding.binding = 0;
		uniformMatLayoutBinding.descriptorCount = 1;
		uniformMatLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformMatLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | 
										VK_SHADER_STAGE_RAYGEN_BIT_KHR 	| VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		layouts[DSL_UNIFORM_BUFFER].bindings = { uniformMatLayoutBinding };
		layouts[DSL_UNIFORM_BUFFER].layout = createDStLayout(layouts[DSL_UNIFORM_BUFFER].bindings);
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
		for (auto & frame_dependent_group : PLayout.descriptors.frame_dependent_sets) {
			for (auto & DSLayout : frame_dependent_group) {
				for (auto & bind : DSLayout.layout->bindings) {
					VkDescriptorPoolSize poolSize;
					poolSize.type = bind.descriptorType;
					poolSize.descriptorCount = bind.descriptorCount * frames_in_flight;
					sets_needed += frames_in_flight;
					poolSizes.push_back(poolSize);
				}
			}
		}
	}

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = sets_needed;
	VkResult result = vkCreateDescriptorPool(Device::get(), &poolInfo, nullptr, &pool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void DescriptorSetsFactory::allocateDescriptorSets(DescSetBundle * bundle)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	auto lambda = [&allocInfo](DescSet *desc_set) {
		allocInfo.descriptorPool = pool; // ONE POOL FOR NOW
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
	VkWriteDescriptorSet descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

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
						PhysicalDevice::getProperties().properties.limits.minUniformBufferOffsetAlignment;
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
		PhysicalDevice::getProperties().properties.limits.minUniformBufferOffsetAlignment;
	VkDeviceSize padding = minAlignement - (sizeof(UniformBlock) % minAlignement);

	memcpy((char *)mappedUniformMemory + imageIndex * (sizeof(UniformBlock) + padding),
		&uniforms, sizeof(uniforms));
}

void DescriptorSetsFactory::cleanUp() {
	vkUnmapMemory(Device::get(), uniformBufferMemory);
	vkDestroyBuffer(Device::get(), uniformBuffer, nullptr);
	vkFreeMemory(Device::get(), uniformBufferMemory, nullptr);

	vkDestroyDescriptorPool(Device::get(), pool, nullptr);

	for (auto & DSlayout : layouts) {
		vkDestroyDescriptorSetLayout(Device::get(), DSlayout.layout, nullptr);
	}
	layouts.clear();
}

void DescriptorSetsFactory::createUniformBuffer()
{
	// one uniform block for each frame in-flight
	VkDeviceSize minAlignement =
		PhysicalDevice::getProperties().properties.limits.minUniformBufferOffsetAlignment;
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
		case DescSetUsage::DS_USAGE_ALBEDO_TEXTURE:
			imagesInfo.resize(SUPPORTED_TEXTURE_COUNT);
			for (int i = 0; i < SUPPORTED_TEXTURE_COUNT; i++) {
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
		case DescSetUsage::DS_USAGE_ALBEDO_TEXTURE:
			imagesInfo.resize(SUPPORTED_TEXTURE_COUNT);
			for (int i = 0; i < SUPPORTED_TEXTURE_COUNT; i++) {
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


