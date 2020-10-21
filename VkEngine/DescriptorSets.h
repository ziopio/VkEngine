#pragma once
#include "commons.h"
#include "LightSource.h"

constexpr const unsigned TEXTURE_ARRAY_LENGTH = 32;
/*
 A library of pre-defined descriptors sets layouts identified by an enumeration
*/
enum DescSetsLayouts {
	// lenght
	DSL_TEXTURE_ARRAY,
	DSL_FRAMEBUFFER_TEXTURE,
	DSL_UNIFORM_BUFFER,
	DSL_RAY_TRACING_SCENE, // Raytracing KHR
	DSL_RT_IMAGE_AND_OBJECTS,
	DescSetsLayouts_END // must be last
};
/* This is needed to choose which group of assets should 
be used to initialize a DescSetBundle. */
enum DescSetsResourceContext {
	SCENE_DATA,
	IMGUI_DATA,
	DescSetsResourceContext_END
};
/* 
Use this enum to descibe the role of a desc set.
Desc sets with the same layout could link to different resources
to serve different purposes inside a shader...
*/
enum DescSetUsage {
	DS_USAGE_UNDEFINED,// when usage is not needed at current developing stage...
	DS_USAGE_ALBEDO_TEXTURE,
	DS_USAGE_NORMAL_MAP,
	DS_OFFSCREEN_RENDER_TARGET,
	// TODO add some more..
};

typedef struct {
	//DescSetsLayouts type;
	VkDescriptorSetLayout layout;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	//bool frame_dependent;//Tells if the set should be replicated for each frame in flight
} DescSetLayout;

typedef struct {
	DescSetUsage purpose;
	DescSetLayout *layout;
	VkDescriptorSet set;
} DescSet;

// This struct groups DescSets of a specifique Pipeline layout
typedef struct {
	// Are we going to load SCENE_DATA or IMGUI_DATA, ecc..
	DescSetsResourceContext data_context;
	// always valid for the pipelines
	std::vector<DescSet> static_sets;
	/* Lists of descriptors sets that are replicated for each frame in flight.
	Example: if the swapchain has 3 images, each set is repeated 3 times 
	to allow rendering of multiple frames on different render targets or textures.
	*/
	std::vector<std::vector<DescSet>> frame_dependent_sets;
} DescSetBundle;

// Used in the Ray-Tracing pipeline
struct SceneObjRtDescBlock {
	uint32_t meshID;
	uint32_t textureID;
	glm::mat4 transform;
};

struct UniformBlock {
	glm::mat4 P_matrix;
	glm::mat4 V_matrix;
	vkengine::LightData lights[10];
	uint32_t light_count;
};

struct MainPushConstantBlock { // max 128 bytes for compatibility reasons
	glm::mat4 model_transform;
	uint32_t textureIndex;
};

struct ImGuiPushConstantBlock {
	glm::vec2 uScale;
	glm::vec2 uTranslate;
	uint32_t tex_ID;
};

class DescriptorSetsFactory {
public:
	static void initLayouts();	
	//
	static void initDescSetPool();
	inline static DescSetLayout * getDescSetLayout(DescSetsLayouts type)
	{	return &DescriptorSetsFactory::layouts[type];	};
	/* Allocates one instance of VkDescriptorSet for each DSLayout specified in the bundle.*/
	static void allocateDescriptorSets(DescSetBundle* bundle);
	static void createUniformBuffer();
	// Depending on the bundle config initializes its VkDescriptorSets with proper Data
	// Note: the data choice logic is embedded in the code, this should change in future...
	static void updateDescriptorSets(DescSetBundle* bundle);
	static void updateUniformBuffer(UniformBlock uniforms, int imageIndex);
	inline static VkBuffer getUniformBuffer() {	return uniformBuffer;};
	static void cleanUp();
private:
	static std::vector<DescSetLayout> layouts;
	static VkDescriptorPool pool;

	// For now this class manages the unique Uniform buffer needed
	static VkBuffer uniformBuffer;
	static VkDeviceMemory uniformBufferMemory;
	static void* mappedUniformMemory;

	static std::vector<VkDescriptorImageInfo> gatherImageInfos(DescSetUsage usage, DescSetsResourceContext data_context);
	//static std::vector<VkDescriptorBufferInfo> gatherBufferInfos(DescSetUsage usage, DescSetsResourceContext data_context);

};