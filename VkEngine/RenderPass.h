#pragma once
#include "SwapChain.h"

class RenderPass
{
public:
	RenderPass(SwapChain* swapchain);
	//VkRenderPass get_ForwardRenderPass();
	//VkRenderPass get_DoubleSubPass_RenderPass();
	VkRenderPass get_SimpleRenderPass();
	VkRenderPass get_OffScreenRenderPass();

	~RenderPass();
private:
	void createForwardRenderPass();
	void createSimpleRenderPass();
	void createOffScreenRenderPass();
	VkRenderPass simpleRP;
	VkRenderPass offscreenRP;
	VkRenderPass forwardRP;
	VkRenderPass deferredRP;
	SwapChain* swapchain;
};

