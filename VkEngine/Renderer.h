#pragma once
#include "RenderPass.h"
#include "Scene3D.h"
#include "LightSource.h"
#include "Libraries/threadpool.hpp"

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
	static void init();
	static unsigned getNextFrameBufferIndex();
	static FrameAttachment getOffScreenFrameAttachment(unsigned frameIndex);
	static void prepareScene(vkengine::Scene3D* scene);
	/* 
	 * true means OK, false means SWAPCHAIN CHANGED!!!
	*/
	static bool renderScene();
	static void cleanUp();
	static bool multithreading;
private:
	static void createFramebuffers();
	static void createOffScreenAttachments();
	static void prepareThreadedRendering();
	static void updateUniforms(uint32_t frameBufferIndex);
	static void updateOffScreenCommandBuffer(uint32_t frameBufferIndex);
	static void updateFinalPassCommandBuffer(uint32_t frameBufferIndex);

	static void recordImGuiDrawCmds(uint32_t frameBufferIndex);
	static void findObjXthreadDivision(unsigned obj_num);
	static void createSyncObjects();

	static FrameAttachment final_depth_buffer;
	static FrameAttachment offScreen_depth_buffer;
	static std::vector<FrameAttachment> offScreenAttachments;


	static std::vector<VkFramebuffer> swapChainFramebuffers;
	static std::vector<VkFramebuffer> offScreenFramebuffers;

	static std::vector<VkSemaphore> imageAvailableSemaphores;
	static std::vector<VkSemaphore> offScreenRenderReadySemaphores;
	static std::vector<VkSemaphore> renderFinishedSemaphores;
	static std::vector<VkFence> inFlightFences;

	static VkCommandPool primaryCommandPool;
	static std::vector<VkCommandBuffer> offScreenCmdBuffers;
	static std::vector<VkCommandBuffer> primaryCmdBuffers;
	//VkCommandPool mainThreadSecondaryCmdPool;// gui records on main thread
	//std::vector<VkCommandBuffer> mainThreadSecondaryCmdBuffers; // for gui

	static vkengine::Scene3D* scene;

	static vks::ThreadPool thread_pool;
	static std::vector<ThreadData> per_thread_resources;

	static uint32_t numThreads;
	static uint32_t objXthread;
	static uint32_t currentFrame;
	static uint32_t last_imageIndex;
};

