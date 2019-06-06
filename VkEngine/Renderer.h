#pragma once
#include "RenderPass.h"
#include "Object.h"
#include "Libraries/threadpool.hpp"
#include "Libraries/frustum.hpp"

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct DepthBufferResource {
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
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
	void createDepthResources();
	void prepareThreadedRendering();
	void update_camera_infos(uint32_t frameBufferIndex);
	void updateCommandBuffer(uint32_t frameBufferIndex);
	void findObjXthreadDivision();
	void createSyncObjects();

	DepthBufferResource depth_buffer;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	RenderPass* renderPass;
	SwapChain* swapChain;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VkCommandPool primaryCommandPool;
	std::vector<VkCommandBuffer> primaryCommandBuffers;
	vks::ThreadPool thread_pool;
	std::vector<ThreadData> per_thread_resources;

	vks::Frustum frustum;
	std::vector<Object> objects;
	std::vector<LightSource> lights;

	uint32_t numThreads;
	uint32_t objXthread;
	uint32_t currentFrame;
};

