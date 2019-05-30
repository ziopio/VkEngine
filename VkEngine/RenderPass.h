#pragma once
#include "SwapChain.h"

class RenderPass
{
public:
	RenderPass(SwapChain* swapchain);
	VkRenderPass get();
	~RenderPass();
private:
	void createRenderPass(SwapChain* swapchain);
	VkRenderPass renderPass;
};

