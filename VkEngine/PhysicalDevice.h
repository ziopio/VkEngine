#pragma once
#include "commons.h"

const std::vector<const char*> requiredDeviceExtensions = {
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
	static VkPhysicalDeviceProperties2& getPhysicalDeviceProperties();
	static VkPhysicalDeviceFeatures2& getPhysicalDeviceFeatures();
	inline static bool hasRaytracing() { return raytracing; };
private:
	static void pickPhysicalDevice();
	static bool isDeviceSuitable(VkPhysicalDevice device);
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	// Checks for extensions, returns true if at least the minimal extensions can be found.
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);	
//Fields	
	static bool ready;
	static VkSurfaceKHR surface;
	static SwapChainSupportDetails swapChainSupportDetails;
	static QueueFamilyIndices queueFamilyIndices;	
	static VkPhysicalDevice physicalDevice;
	// Basic Vulkan properties and features
	static VkPhysicalDeviceProperties basicProperties;
	static VkPhysicalDeviceFeatures basicFeatures;
	// Structs concatenating all features and properties of the device
	static VkPhysicalDeviceProperties2 deviceProperties2;
	static VkPhysicalDeviceFeatures2 deviceFeatures2;
	// Extra features and properties
	// Buffer Device Address
	static VkPhysicalDeviceBufferDeviceAddressFeaturesKHR deviceAddrFeatures;
	// Ray Tracing
	static VkPhysicalDeviceRayTracingPropertiesKHR rayTracingProperties;
	static VkPhysicalDeviceRayTracingFeaturesKHR rayTracingFeatures;
	static bool raytracing;
};

