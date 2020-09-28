#pragma once
#include "commons.h"

class RenderPassCatalog
{
public:
	static VkRenderPass presentationRP;
	static VkRenderPass offscreenRP;
	static void init();
	static void cleanUP();
private:
	//static void createForwardRenderPass();
	static void createPresentationRenderPass();
	static void createOffScreenRenderPass();
};

