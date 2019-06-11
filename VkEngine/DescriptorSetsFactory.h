#pragma once
#include "MaterialManager.h"
#include "Object.h"


class DescriptorSetsFactory
{
public:
	static void init(SwapChain* swapChain);
	static void updateUniformBuffer(uniformBlockDefinition uniforms, int imageIndex);	
	static VkDescriptorSet getFrameDescriptorSet(int frame_index);
	static VkDescriptorSet getMaterialDescriptorSets();
	static VkDescriptorSet getStaticGlobalDescriptorSet();
	static VkDescriptorSet getImGuiDescriptorSet();
	static void cleanUp();
private:
	static void createDescriptorSets();
	static void createDescriptorPool();
	static void createFrameDependentUniformBuffers();
	static SwapChain* swapChain;
	static std::multimap<MaterialType, Object*> material2obj_map;
	static VkDescriptorPool descriptorPool;
	// Descriptors Sets that belong to a specific "in-flight" frame
	static std::vector<VkDescriptorSet> frameDescriptorSets;
	// Descriptors Sets that belong to a specific material
	static std::vector<VkDescriptorSet> materialDescriptorSets;
	// A Destriptor Set common to all pipeline layouts 
	static VkDescriptorSet staticGlobalDescriptorSet;
	// The descriptor set used to store imgSamplers for the gui
	static VkDescriptorSet imGuiDescriptorSet;
	static void* mappedUniformMemory;
	//One buffer used with offsets from the frameDescriptorSets
	static VkBuffer uniformBuffers;
	static VkDeviceMemory uniformBuffersMemory;
	static bool ready;
};

