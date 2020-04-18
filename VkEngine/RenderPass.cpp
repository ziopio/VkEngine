#include "stdafx.h"
#include "ApiUtils.h"
#include "RenderPass.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "SwapChain.h"


VkRenderPass RenderPassCatalog::presentationRP;
VkRenderPass RenderPassCatalog::offscreenRP;

void RenderPassCatalog::init()
{
	//createForwardRenderPass();
	createPresentationRenderPass();
	createOffScreenRenderPass();
}

void RenderPassCatalog::cleanUP()
{
	vkDestroyRenderPass(Device::get(), presentationRP, nullptr);
	vkDestroyRenderPass(Device::get(), offscreenRP, nullptr);
}


void RenderPassCatalog::createPresentationRenderPass()
{
	// Attachment description for the whole renderpass
	std::array<VkAttachmentDescription, 2> attachments = {};
	//Final color attachment
	VkAttachmentDescription outPutColorAttachment = {};
	outPutColorAttachment.format = SwapChainMng::get()->getFormat();
	outPutColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	outPutColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // i want a black screen
	outPutColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // i want to save the render
	outPutColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // no stencil
	outPutColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // no stencil
	outPutColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	outPutColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // final render ready to be presented
	// Depth attachment
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat(PhysicalDevice::get());
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments = { outPutColorAttachment, depthAttachment };

	// Just one fSubPass
	std::array<VkSubpassDescription, 1>  subpassDescriptions = {};

	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }; // Final color
	//reference to the depth attachment
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1; // index in the attachments array
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptions[0].colorAttachmentCount = 1;
	subpassDescriptions[0].pColorAttachments = &colorReference;
	subpassDescriptions[0].pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(Device::get(), &renderPassInfo, nullptr, &presentationRP) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void RenderPassCatalog::createOffScreenRenderPass()
{
	// Attachment description for the whole renderpass
	std::array<VkAttachmentDescription, 2> attachments = {};
	//Final color attachment
	VkAttachmentDescription outPutColorAttachment = {};
	outPutColorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
	outPutColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	outPutColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // i want a black screen
	outPutColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // i want to save the render
	outPutColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // no stencil
	outPutColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // no stencil
	outPutColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	outPutColorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // OutPut ready to be read by shader
	// Depth attachment
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat(PhysicalDevice::get());
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments = { outPutColorAttachment, depthAttachment };

	// Just one fSubPass
	std::array<VkSubpassDescription, 1>  subpassDescriptions = {};

	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }; // Final color
	//reference to the depth attachment
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1; // index in the attachments array
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptions[0].colorAttachmentCount = 1;
	subpassDescriptions[0].pColorAttachments = &colorReference;
	subpassDescriptions[0].pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkSubpassDependency, 2>   dependencies = {};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0; // index of next subpass
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	renderPassInfo.pSubpasses = subpassDescriptions.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(Device::get(), &renderPassInfo, nullptr, &offscreenRP) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

//
//void RenderPassCatalog::createForwardRenderPass()
//{
//	// Attachment description for the whole renderpass
//	std::array<VkAttachmentDescription, 3> attachments = {};
//	// Final color attachment
//	VkAttachmentDescription outPutColorAttachment = {};
//	outPutColorAttachment.format = swapchain->getFormat();
//	outPutColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	outPutColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // i want a black screen
//	outPutColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // i want to save the render
//	outPutColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // no stencil
//	outPutColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // no stencil
//	outPutColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	outPutColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // final render ready to be presented
//	// Offscreen preliminar attachment to use the 3D scene rendering as texture later on
//	VkAttachmentDescription offscreenColorAttachment = {};
//	offscreenColorAttachment.format = swapchain->getFormat();
//	offscreenColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	offscreenColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // i want a black screen
//	offscreenColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // i want to save the render
//	offscreenColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // no stencil
//	offscreenColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // no stencil
//	offscreenColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	offscreenColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//	// Depth attachment
//	VkAttachmentDescription depthAttachment = {};
//	depthAttachment.format = findDepthFormat(PhysicalDevice::get());
//	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//	attachments = { outPutColorAttachment, offscreenColorAttachment, depthAttachment };
//
//	// Two subpasses
//	std::array<VkSubpassDescription, 2>  subpassDescriptions = {};
//
//	// First subpass: 3D scene rendering on offscreen attachment
//	// ----------------------------------------------------------------------------------------
//	VkAttachmentReference offScreenColorReferences = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }; // Offscreen color WRITE
//	//reference to the depth attachment
//	VkAttachmentReference depthAttachmentRef = {};
//	depthAttachmentRef.attachment = 2; // index in the attachments array
//	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//	subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpassDescriptions[0].colorAttachmentCount = 1;
//	subpassDescriptions[0].pColorAttachments = &offScreenColorReferences;
//	subpassDescriptions[0].pDepthStencilAttachment = &depthAttachmentRef;
//
//	// Second subpass: Final composition (using G-Buffer components)
//	// ----------------------------------------------------------------------------------------
//	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }; // Final color
//	VkAttachmentReference inputReference = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }; // Offscreen color READ
//	subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//	subpassDescriptions[1].colorAttachmentCount = 1;
//	subpassDescriptions[1].pColorAttachments = &colorReference;
//	subpassDescriptions[1].pDepthStencilAttachment = &depthAttachmentRef;
//	// Use the color/depth attachments filled in the first pass as input attachments
//	subpassDescriptions[1].inputAttachmentCount = 1;
//	subpassDescriptions[1].pInputAttachments = &inputReference;
//
//
//	std::array<VkSubpassDependency, 3>   dependencies = {};
//	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//	dependencies[0].dstSubpass = 0; // index of next subpass
//	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	// This dependency transitions the offscreen input attachment from color attachment to shader read
//	dependencies[1].srcSubpass = 0;
//	dependencies[1].dstSubpass = 1; // index of next subpass
//	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	dependencies[2].srcSubpass = 0;
//	dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;; // index of next subpass
//	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//	dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
//	dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
//	dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//	VkRenderPassCreateInfo renderPassInfo = {};
//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//	renderPassInfo.pAttachments = attachments.data();
//	renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
//	renderPassInfo.pSubpasses = subpassDescriptions.data();
//	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
//	renderPassInfo.pDependencies = dependencies.data();
//
//	if (vkCreateRenderPass(Device::get(), &renderPassInfo, nullptr, &forwardRP) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create render pass!");
//	}
//}
//
