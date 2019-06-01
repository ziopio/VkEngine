#pragma once

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices {
	// -1 = "not found"
	int graphicsFamily = -1; // the queue index for graphic commands
	int presentFamily = -1; //  the queue index for presentation commands
	int transferFamily = -1; // the queu index for data transfer commands

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0 && transferFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class PhysicalDevice
{
public:
	static void setSurface(VkSurfaceKHR surface);
	static VkSurfaceKHR getSurface();
	static VkPhysicalDevice get();
	static QueueFamilyIndices getQueueFamilies();
	static SwapChainSupportDetails getSwapChainSupport();
private:
	static void pickPhysicalDevice();
	static bool isDeviceSuitable(VkPhysicalDevice device);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	static VkPhysicalDevice physicalDevice;
	static VkSurfaceKHR surface;
	static SwapChainSupportDetails swapChainSupportDetails;
	static QueueFamilyIndices queueFamilyIndices;
	static bool ready;
};

