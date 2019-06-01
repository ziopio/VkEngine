#pragma once
#include "MaterialManager.h"
#include "Object.h"


class DescriptorSetsFactory
{
public:
	static void init(SwapChain* swapChain, std::vector<Object*> objects);
	static VkDescriptorSet* getDescriptorSet(MaterialType material);
	static void updateUniformBuffer(uniBlock uniforms, int imageIndex);
	static void cleanUp();
private:
	static void mapMaterialsToObjects();
	static void createDescriptorSets();
	static void createDescriptorPool();
	static void createUniformBuffers();
	static SwapChain* swapChain;
	static std::vector<Object*> objects;
	static std::multimap<MaterialType, Object*> material2obj_map;
	static VkDescriptorPool descriptorPool;
	static std::vector<VkDescriptorSet> frameDescriptorSets;
	static std::vector<VkDescriptorSet> staticDescriptorSets;
	static std::vector<void *> mappedUniformMemory;
	static std::vector<VkBuffer> uniformBuffers;
	static std::vector<VkDeviceMemory> uniformBuffersMemory;
	static bool ready;
};

