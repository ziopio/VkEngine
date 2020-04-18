#pragma once
#include "VkEngine.h"
#include "Surface.h"

class SwapChainMng {
	class SwapChain {
	public:
		SwapChain(vkengine::SurfaceOwner* surface_owner);
		bool acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex);
		bool presentImage(uint32_t imageIndex, VkSemaphore* semaphores);
		inline VkSwapchainKHR get() { return this->swapChain; };
		inline unsigned getImageCount() { return this->image_count; };
		inline std::vector<VkImageView> getImageViews() {return this->swapImageViews;}
		inline VkFormat getFormat() {return this->swapChainImageFormat;	};
		inline VkExtent2D getExtent() { return  this->swapChainExtent; };
		~SwapChain();
	private:
		void createSwapChain();
		void createImageViews();
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		vkengine::SurfaceOwner* surface_owner;
		VkSwapchainKHR swapChain;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImage> swapImages;
		std::vector<VkImageView> swapImageViews;
		unsigned image_count;
	};
public:
	static void init(vkengine::SurfaceOwner* surface_owner);
	static SwapChainMng::SwapChain* get();
	static void cleanUP();
private:
	static std::vector<SwapChain*> swapchains;
	static unsigned current_swapchain;
};
