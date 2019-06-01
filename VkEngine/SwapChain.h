#pragma once
#include "VkEngine.h"
#include "Surface.h"

class SwapChain
{
public:
	SwapChain();
	bool acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex);
	bool presentImage(uint32_t imageIndex, VkSemaphore* semaphores);
	VkSwapchainKHR get();
	std::vector<VkImageView> getImageViews();
	VkFormat getFormat();
	VkExtent2D getExtent();
	~SwapChain();
private:
	void createSwapChain();
	void createImageViews();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImage> swapImages;
	std::vector<VkImageView> swapImageViews;
};

