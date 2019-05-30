#pragma once
#include "MaterialManager.h"
#include "Object.h"


class DescriptorSetsFactory
{
public:
	static void init(SwapChain* swapChain, std::vector<Object*> objects);
	static VkDescriptorSet* getDescriptorSet(MaterialType material);
	static void updateUniformBuffer(uniBlock uniforms);
	static void cleanUp();
private:
	static void mapMaterialsToObjects();
	static void createDescriptorSets();
	static void createDescriptorPool();
	static void createUniformBuffer();
	static SwapChain* swapChain;
	static std::vector<Object*> objects;
	static std::multimap<MaterialType, Object*> material2obj_map;
	static VkDescriptorPool descriptorPool;
	static std::vector<VkDescriptorSet> descriptorSets;
	static void* mappedUniformMemory;
	static VkBuffer uniformBuffer;
	static VkDeviceMemory uniformBuffersMemory;
	static bool ready;
};

