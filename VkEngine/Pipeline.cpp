#include "Pipeline.h"
#include "DescriptorSets.h"
#include "Device.h"
#include "Mesh.h"

std::unordered_map<std::string, Pipeline> PipelineFactory::pipelines;
std::vector<PipelineLayout> PipelineFactory::pipeline_layouts;
std::string PipelineFactory::current_pipeline;

void PipelineFactory::init()
{
	DescriptorSetsFactory::initLayouts();
	PipelineFactory::createPipelineLayouts();
	DescriptorSetsFactory::initDescSetPool();
	for (auto & layout : pipeline_layouts) {
		DescriptorSetsFactory::allocateDescriptorSets(&layout.descriptors);
	};	
	// TODO refactor this action in a more suited method and place
	DescriptorSetsFactory::createUniformBuffer();
}

void PipelineFactory::newPipeline(const char * name,
	VkRenderPass *renderpass, unsigned subpass_index, PipelineLayoutType type)
{
	PipelineFactory::current_pipeline = name;
	pipelines[name] = {};
	pipelines[name].layout_type = type;
	pipelines[name].render_pass = renderpass;
	pipelines[name].subpass_index = subpass_index;
	pipelines[name].settings.pipeline_layout = &PipelineFactory::pipeline_layouts[type].layout;

	initializeWithDefaultSettings(&pipelines[name].settings);
}

void PipelineFactory::setVertexType(VertexTypes type)
{
	auto setup = &pipelines[current_pipeline].settings;
	//Chiedo alla struct vertex info sul binding e descrizioni sugli attributi
	switch (type)
	{
	case VERTEX_2D:		
		setup->bindingDescriptions = { Vertex2D::getBindingDescription() };
		setup->attributeDescriptions = Vertex2D::getAttributeDescriptions();
		break;
	case VERTEX_3D:
		setup->bindingDescriptions = { Vertex3D::getBindingDescription() };
		setup->attributeDescriptions = Vertex3D::getAttributeDescriptions();
		break;
	default:
		break;
	}	
	// vertex buffer setup to feed the vertex shader
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1; // for instancing
	vertexInputInfo.pVertexBindingDescriptions = setup->bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(setup->attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = setup->attributeDescriptions.data();

	setup->vertexInputInfo = vertexInputInfo;
}

void PipelineFactory::setShaders(const char * vertex, const char * fragment)
{
	VulkanPipelineSettings* setup = &pipelines[current_pipeline].settings;
	setup->vertex = std::make_unique<Shader>(vertex, VK_SHADER_STAGE_VERTEX_BIT);
	setup->fragment = std::make_unique<Shader>(fragment, VK_SHADER_STAGE_FRAGMENT_BIT);

	setup->shader_stages = { setup->vertex->getStage(), setup->fragment->getStage() };
}

void PipelineFactory::setDepthTest(bool flag)
{
	VulkanPipelineSettings* setup = &pipelines[current_pipeline].settings;
	setup->depthStencil.depthTestEnable = flag ? VK_TRUE : VK_FALSE;
}

void PipelineFactory::setCulling(bool flag)
{
	VulkanPipelineSettings* setup = &pipelines[current_pipeline].settings;
	setup->rasterizer.cullMode = flag ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
}

void PipelineFactory::setDynamicViewPortAndScissor()
{
	VulkanPipelineSettings* setup = &pipelines[current_pipeline].settings;
	setup->dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = setup->dynamic_states.data();
	setup->dynamic_states_info = dynamic_state;
}

void PipelineFactory::createRasterizationPipelines()
{
	std::vector<VkPipeline> pipeline_objects(pipelines.size());
	std::vector<Pipeline*> pipeline_references; // pointers to values of the map
	pipeline_references.reserve(pipelines.size());
	std::vector<VkGraphicsPipelineCreateInfo> create_infos;
	create_infos.reserve(pipelines.size());

	std::for_each(pipelines.begin(), pipelines.end(), 
		[&create_infos, &pipeline_references] (std::pair<const std::string,Pipeline>& entry)
	{
		Pipeline* p = &entry.second;
		pipeline_references.push_back(p); // saving pointer

		VkGraphicsPipelineCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.stageCount = p->settings.shader_stages.size();
		info.pStages = p->settings.shader_stages.data();
		info.pVertexInputState = &p->settings.vertexInputInfo;
		info.pInputAssemblyState = &p->settings.inputAssembly;
		info.pViewportState = &p->settings.viewportState;
		info.pRasterizationState = &p->settings.rasterizer;
		info.pMultisampleState = &p->settings.multisampler;
		info.pDepthStencilState = &p->settings.depthStencil; // Optional
		info.pColorBlendState = &p->settings.colorBlending;
		if (p->settings.dynamic_states.size() > 0) 
		{
			info.pDynamicState = &p->settings.dynamic_states_info;
		}else
		{ 
			info.pDynamicState = nullptr; 
		}
		info.layout = *p->settings.pipeline_layout;
		info.renderPass = *p->render_pass;
		info.subpass = p->subpass_index; // the index of the subpass in which the graphic pipeline will be used
		info.basePipelineHandle = VK_NULL_HANDLE; // Optional
		info.basePipelineIndex = -1; // Optional

		create_infos.push_back(info);
	});
	if (vkCreateGraphicsPipelines(Device::get(), VK_NULL_HANDLE, create_infos.size(), create_infos.data(),
		nullptr, pipeline_objects.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	// copia di ogni VkPipeline nella mappa
	for (int i = 0; i < pipelines.size(); i++) {
		pipeline_references[i]->pipeline = pipeline_objects[i];
	}
}

void PipelineFactory::updatePipelinesViewPorts()
{
	for (std::pair<const std::string, Pipeline> & entry : PipelineFactory::pipelines) {
		vkDestroyPipeline(Device::get(), entry.second.pipeline, nullptr);
		for (auto & viewport : entry.second.settings.viewports) {
			viewport.width = (float)SwapChainMng::get()->getExtent().width;
			viewport.height = (float)SwapChainMng::get()->getExtent().height;
		}
		for (auto & scissor : entry.second.settings.scissors) {
			scissor.extent = SwapChainMng::get()->getExtent();
		}
	}
	// TODO: Optimize with use of pipelineCaches/derivative ecc
	PipelineFactory::createRasterizationPipelines();
}

void PipelineFactory::updatePipelineResources(PipelineLayoutType PLayout)
{
	DescriptorSetsFactory::updateDescriptorSets(&pipeline_layouts[PLayout].descriptors);
}

void PipelineFactory::cleanUP()
{
	for (std::pair<const std::string,Pipeline> & entry : PipelineFactory::pipelines) {
		vkDestroyPipeline(Device::get(), entry.second.pipeline, nullptr);
	}
	pipelines.clear();
	for (auto & Playout : pipeline_layouts) {
		vkDestroyPipelineLayout(Device::get(), Playout.layout, nullptr);
	}
	pipeline_layouts.clear();
}

void PipelineFactory::initializeWithDefaultSettings(VulkanPipelineSettings * setup)
{
	// As default the pipeline is intended for 3D vertices
	setup->bindingDescriptions = { Vertex3D::getBindingDescription() };
	setup->attributeDescriptions = Vertex3D::getAttributeDescriptions();
	// vertex buffer setup to feed the vertex shader
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1; // for instancing
	vertexInputInfo.pVertexBindingDescriptions = setup->bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(setup->attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = setup->attributeDescriptions.data();

	setup->vertexInputInfo = vertexInputInfo;

	// definizione dell'imput della pipeline
	// Ovvero come interpretare l'ordine dei vertici per il disegno
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // tipo di disegno
	inputAssembly.primitiveRestartEnable = VK_FALSE; // VK_TRUE enables special index for mesh splitting during drawing
	setup->inputAssembly = inputAssembly;

	//il viewport definisce quale porzione del frame è occupato dall'immagine
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)SwapChainMng::get()->getExtent().width;
	viewport.height = (float)SwapChainMng::get()->getExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	setup->viewports = { viewport };

	// il rettangolo che seleziona la parte dell'immagine da rasterizzare sul frame
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = SwapChainMng::get()->getExtent();
	setup->scissors = { scissor };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = setup->viewports.size();
	viewportState.pViewports = setup->viewports.data();
	viewportState.scissorCount = setup->scissors.size();
	viewportState.pScissors = setup->scissors.data();
	setup->viewportState = viewportState;

	// Enabled by default
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;// play on these for transparency drawing
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // lower depth means near
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional
	setup->depthStencil = depthStencil;

	// Definizione del rasterizer per la creazione dei fragments
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // alternatives are _LINE or _POINT
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//if (this->type == MaterialType::UI) {
	//	rasterizer.cullMode = VK_CULL_MODE_NONE;
	//}
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	setup->rasterizer = rasterizer;

	//Definizione del multisampler
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional
	setup->multisampler = multisampling;

	//Definizione del blending per 1 framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	setup->colorBlendAttachments = { colorBlendAttachment };

	//Definizione del blending globale
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;  // enable for color combination (this overwrites blendEnable for every framebuffer) 
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = setup->colorBlendAttachments.size();
	colorBlending.pAttachments = setup->colorBlendAttachments.data(); // info specifiche per ogni framebuffer
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional
	setup->colorBlending = colorBlending;
}

//VkPipelineLayout createPipelineLayout() 
//{
//	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
//	pipelineLayoutInfo.setLayoutCount = layouts.size();
//	pipelineLayoutInfo.pSetLayouts = layouts.data();
//}

void PipelineFactory::createPipelineLayouts()
{
	PipelineFactory::pipeline_layouts.resize(PipelineLayoutType::PipelineLayoutType_END);

	// STD rendering pipeline layout---------------------------------------	
	{
		std::vector<VkDescriptorSetLayout> layouts;
		layouts.push_back(DescriptorSetsFactory::getDescSetLayout(DSL_TEXTURE_ARRAY)->layout);
		layouts.push_back(DescriptorSetsFactory::getDescSetLayout(DSL_UNIFORM_BUFFER)->layout);
		VkPushConstantRange pushRange = {};
		pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushRange.size = sizeof(MainPushConstantBlock);
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
		pipelineLayoutInfo.pPushConstantRanges = &pushRange; // Optional
		if (vkCreatePipelineLayout(Device::get(),
			&pipelineLayoutInfo, nullptr,
			&PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_STANDARD].layout)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
		DescSetBundle bundle = {};
		//bundle configuration
		bundle.static_sets.push_back(
			{ DS_USAGE_ALBEDO_TEXTURE, DescriptorSetsFactory::getDescSetLayout(DSL_TEXTURE_ARRAY),nullptr });
		bundle.frame_dependent_sets.push_back({});
		for (int i = 0; i < SwapChainMng::get()->getImageCount(); i++) {
			bundle.frame_dependent_sets[0].push_back(
				{ DS_USAGE_UNDEFINED, DescriptorSetsFactory::getDescSetLayout(DSL_UNIFORM_BUFFER),nullptr });
		}
		bundle.data_context = DescSetsResourceContext::SCENE_DATA;
		PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_STANDARD].descriptors = bundle;
	}
	

	// Ray_TRACING rendering pipeline layout---------------------------------------	
	{		
		std::vector<VkDescriptorSetLayout> layouts;
		layouts.push_back(DescriptorSetsFactory::getDescSetLayout(DSL_RAY_TRACING_SCENE)->layout); // ray-tracing only
		layouts.push_back(DescriptorSetsFactory::getDescSetLayout(DSL_RT_IMAGE_AND_OBJECTS)->layout); // shared output with rasterizer
		layouts.push_back(DescriptorSetsFactory::getDescSetLayout(DSL_UNIFORM_BUFFER)->layout); // shared input with rasterizer
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		if (vkCreatePipelineLayout(Device::get(),
			&pipelineLayoutInfo, nullptr,
			&PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_RAY_TRACING].layout)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
		//bundle configuration
		DescSetBundle bundle = {};
		bundle.static_sets.push_back(
			{ DS_USAGE_UNDEFINED, DescriptorSetsFactory::getDescSetLayout(DSL_RAY_TRACING_SCENE),nullptr });
		//bundle.static_sets.push_back(
		//	{ DS_USAGE_ALBEDO_TEXTURE, DescriptorSetsFactory::getDescSetLayout(DSL_TEXTURE_ARRAY),nullptr });

		bundle.frame_dependent_sets.push_back({});
		for (int i = 0; i < SwapChainMng::get()->getImageCount(); i++) {
			bundle.frame_dependent_sets[0].push_back(
				{ DS_USAGE_UNDEFINED, DescriptorSetsFactory::getDescSetLayout(DSL_RT_IMAGE_AND_OBJECTS),nullptr });
		}		
		bundle.frame_dependent_sets.push_back({});
		for (int i = 0; i < SwapChainMng::get()->getImageCount(); i++) {
			bundle.frame_dependent_sets[1].push_back(
				{ DS_USAGE_UNDEFINED, DescriptorSetsFactory::getDescSetLayout(DSL_UNIFORM_BUFFER),nullptr });
		}
		bundle.data_context = DescSetsResourceContext::SCENE_DATA;
		PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_RAY_TRACING].descriptors = bundle;

	}
	
	// ImGui rendering pipeline layout-------------------------------------------
	{
		std::vector<VkDescriptorSetLayout> layouts;		
		VkPushConstantRange pushRange = {};
		pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushRange.size = sizeof(ImGuiPushConstantBlock);
		layouts.push_back(DescriptorSetsFactory::getDescSetLayout(DSL_TEXTURE_ARRAY)->layout);
		layouts.push_back(DescriptorSetsFactory::getDescSetLayout(DSL_FRAMEBUFFER_TEXTURE)->layout);
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
		pipelineLayoutInfo.pPushConstantRanges = &pushRange; // Optional
		if (vkCreatePipelineLayout(Device::get(),
			&pipelineLayoutInfo, nullptr,
			&PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_IMGUI].layout)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
		//bundle configuration
		DescSetBundle bundle = {};
		bundle.static_sets.push_back(
			{ DescSetUsage::DS_USAGE_ALBEDO_TEXTURE,DescriptorSetsFactory::getDescSetLayout(DescSetsLayouts::DSL_TEXTURE_ARRAY),nullptr });
		bundle.frame_dependent_sets.push_back({});
		for (int i = 0; i < SwapChainMng::get()->getImageCount(); i++) {
			bundle.frame_dependent_sets[0].push_back(
				{ DescSetUsage::DS_OFFSCREEN_RENDER_TARGET,DescriptorSetsFactory::getDescSetLayout(DescSetsLayouts::DSL_FRAMEBUFFER_TEXTURE),nullptr });
		}
		bundle.data_context = DescSetsResourceContext::IMGUI_DATA;
		PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_IMGUI].descriptors = bundle;
	}


}
