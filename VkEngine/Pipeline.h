#pragma once
#include "VkEngine.h"
#include "Shader.h"
#include "SwapChain.h"
#include "DescriptorSets.h"
#include "Mesh.h"
#include "commons.h"

constexpr const char* STD_3D_PIPELINE_ID = "standard";
constexpr const char* IMGUI_PIPELINE_ID = "imgui";

/*
A set of predefined pipelines Layouts used inside the engine.
*/
enum PipelineLayoutType {
	STD_PIPELINE_LAYOUT,
	IMGUI_PIPELINE_LAYOUT,
	PipelineLayoutType_END
};

typedef struct {
	//enum PipelineLayoutType type;
	VkPipelineLayout layout;
	DescSetBundle descriptors;
} PipelineLayout;
/*
A pipeline is distinguished for compatibility from its layout.
It is created for a specific Subpass of a Render Pass.
*/
typedef struct {
	std::unique_ptr<Shader> vertex, fragment;
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	std::vector<VkViewport> viewports;
	std::vector<VkRect2D> scissors;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo multisampler;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	VkPipelineColorBlendStateCreateInfo colorBlending;
	VkPipelineLayout* pipeline_layout;
	std::vector<VkDynamicState> dynamic_states;
	VkPipelineDynamicStateCreateInfo dynamic_states_info;

	VkGraphicsPipelineCreateInfo pipelineInfo;
} VulkanPipelineSettings;

typedef struct {
	VkPipeline pipeline;
	PipelineLayoutType layout_type;
	VulkanPipelineSettings settings;
	VkRenderPass *render_pass;
	unsigned subpass_index;
} Pipeline;

/*
This static class has the task of specify methods to create pipelines and build theme in one shot.
*/
class PipelineFactory {
public:
	static std::unordered_map<std::string, Pipeline> pipelines;
	static std::vector<PipelineLayout> pipeline_layouts;
	// Call to create the Pipeline layouts
	static void init();
	// Adds a new pipeline definition to the creation list, has a default config, but shaders must always be specified
	static void newPipeline(const char* name, VkRenderPass *renderpass, unsigned subpass_index, PipelineLayoutType type);
	// Example: 2D vertex input vs 3D vertex input
	static void setVertexType(VertexTypes type);
	// The selected shaders have to be compliant with the pipeline layout!
	static void setShaders(const char* vertex, const char* fragment);
	// default is set to true
	static void setDepthTest(bool flag);
	// static void enableStencil();
	// default should be culling set to back
	static void setCulling(bool flag);
	// This is used for imgui 
	static void setDynamicViewPortAndScissor();
	// creates all the pipelines added
	static void createPipelines();
	// Recreates all pipelines with the current swapchain extent,should be called after swapchain recreation.
	static void updatePipelinesViewPorts();
	// Updates all decriptors of a pipeline layout
	static void updatePipelineResources(PipelineLayoutType PLayout);
	// destroyes everything reverting state previous to init()
	static void cleanUP();
private:
	static void initializeWithDefaultSettings(VulkanPipelineSettings * setup);
	static void createPipelineLayouts();
	//static std::unordered_map<std::string, Shader> shaders;
	static std::string current_pipeline;
};
