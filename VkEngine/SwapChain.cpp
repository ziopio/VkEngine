#include "stdafx.h"
#include "SwapChain.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "ApiUtils.h"
#include "VkEngine.h"

std::vector<SwapChainMng::SwapChain*> SwapChainMng::swapchains;
unsigned SwapChainMng::current_swapchain;

SwapChainMng::SwapChain::SwapChain(vkengine::SurfaceOwner * surface_owner)
{
	this->surface_owner = surface_owner;
	this->createSwapChain();
	this->createImageViews();
	this->image_count = this->getImageViews().size();
}

bool SwapChainMng::SwapChain::acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex)
{
	VkResult result = vkAcquireNextImageKHR(Device::get(), this->swapChain, std::numeric_limits<uint64_t>::max(), semaphore, VK_NULL_HANDLE, imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	return true;
}

bool SwapChainMng::SwapChain::presentImage(uint32_t imageIndex, VkSemaphore* semaphores)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = semaphores; // aspetto il semaforo di fine rendering

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	VkResult result = vkQueuePresentKHR(Device::getPresentQueue(), &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		return false;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	return true;
}


SwapChainMng::SwapChain::~SwapChain()
{
	for (auto imageView : swapImageViews) {
		vkDestroyImageView(Device::get(), imageView, nullptr);
	}
	vkDestroySwapchainKHR(Device::get(), swapChain, nullptr);
	swapImages.clear();
	swapImageViews.clear();
}

void  SwapChainMng::SwapChain::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = PhysicalDevice::getSwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// Rimane da scegliere quante immagini può contenere la swapchain
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	// maxImageCount == 0 significa infinite immagini
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = PhysicalDevice::getSurface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // increase for streoscopic 3D only
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = PhysicalDevice::getQueueFamilies();
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily) { // ho 2 code diverse per rendering e presentazione
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // immagine accesibile da più code
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //esclusiva (best performance)
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // Nessuna pre-trasformazione
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // disabilito il blending con l'anmbiente windows
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // se la mia finestra è in parte coperta, disabilita i pixels coperti
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(Device::get(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(Device::get(), swapChain, &imageCount, nullptr);
	this->swapImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Device::get(), swapChain, &imageCount, this->swapImages.data());
	this->swapChainImageFormat = surfaceFormat.format;
	this->swapChainExtent = extent;
}

VkSurfaceFormatKHR  SwapChainMng::SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	//scelta di default se la superficie non ha preferenze
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	// altrimenti cerco la soluzione che combacia con la mia
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	// Se non ci sono match seleziono il primo formato disponibile
	return availableFormats[0];
}

VkPresentModeKHR  SwapChainMng::SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
	VkPresentModeKHR defaultMode = VK_PRESENT_MODE_FIFO_KHR;
	// Se disponibile seleziono la modalità a triplo buffering che è la migliore
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		// I prefer not to use immediate for now
		//else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		//	defaultMode = availablePresentMode;
		//}
	}
	return defaultMode;
}

VkExtent2D SwapChainMng::SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	// Alcuni windowManagers consentono di impostare qualsiasi risoluzione per una finestra
	// ciò accade se i "currentExtent sono a max uint32_t"
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width=0, height=0;
		this->surface_owner->getFrameBufferSize( &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}


void SwapChainMng::SwapChain::createImageViews() {
	swapImageViews.resize(swapImages.size());

	for (uint32_t i = 0; i < swapImages.size(); i++) {
		swapImageViews[i] = createImageView(Device::get(), swapImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void SwapChainMng::init(vkengine::SurfaceOwner * surface_owner)
{
	SwapChainMng::swapchains.push_back(new SwapChain(surface_owner));
	current_swapchain = 0;
}

SwapChainMng::SwapChain * SwapChainMng::get()
{
	return SwapChainMng::swapchains[current_swapchain];
}

void SwapChainMng::cleanUP()
{
	for (auto chain : swapchains) {
		delete chain;
	}
	SwapChainMng::swapchains.clear();
}
