#pragma once
#include "RenderPass.h"
#include "Object.h"
#include "LightSource.h"
#include "Libraries/threadpool.hpp"
#include "Libraries/frustum.hpp"

using namespace vkengine;

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct FrameAttachment {
	VkImage image;
	VkDeviceMemory Memory;
	VkImageView imageView;
	VkSampler Sampler;
};

struct ThreadData {
	// One pool per thread
	VkCommandPool commandPool;
	// One secondary command buffer per object per framebuffer
	std::vector<std::vector<VkCommandBuffer>> commandBuffers;
};

class Renderer
{
public:
	Renderer(RenderPass* renderPass,SwapChain* swapChain);
	unsigned getNextFrameBufferIndex();
	FrameAttachment getOffScreenFrameAttachment(unsigned frameIndex);
	void setLights(std::vector<LightSource> lights);
	void setObjects(std::vector<Object> objects);
	/* 
	 * true means OK, false means SWAPCHAIN CHANGED!!!
	*/
	bool renderScene();
	~Renderer();
	bool multithreading;
private:
	void createFramebuffers();
	void createOffScreenAttachments();
	void prepareThreadedRendering();
	void updateUniforms(uint32_t frameBufferIndex);
	void updateOffScreenCommandBuffer(uint32_t frameBufferIndex);
	void updateFinalPassCommandBuffer(uint32_t frameBufferIndex);

	void recordImGuiDrawCmds(uint32_t frameBufferIndex);
	void findObjXthreadDivision();
	void createSyncObjects();

	FrameAttachment final_depth_buffer;
	FrameAttachment offScreen_depth_buffer;
	std::vector<FrameAttachment> offScreenAttachments;


	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkFramebuffer> offScreenFramebuffers;

	RenderPass* renderPass;
	SwapChain* swapChain;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> offScreenRenderReadySemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VkCommandPool primaryCommandPool;
	std::vector<VkCommandBuffer> offScreenCmdBuffers;
	std::vector<VkCommandBuffer> primaryCmdBuffers;
	//VkCommandPool mainThreadSecondaryCmdPool;// gui records on main thread
	//std::vector<VkCommandBuffer> mainThreadSecondaryCmdBuffers; // for gui

	vks::ThreadPool thread_pool;
	std::vector<ThreadData> per_thread_resources;

	vks::Frustum frustum;
	std::vector<Object> objects;
	std::vector<LightSource> lights;

	uint32_t numThreads;
	uint32_t objXthread;
	uint32_t currentFrame;
	uint32_t last_imageIndex;
};

