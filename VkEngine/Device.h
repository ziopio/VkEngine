#pragma once
#include "commons.h"

struct Buffer {
	VkBuffer vkBuffer;
	VkDeviceMemory vkMemory;
	VkDeviceAddress deviceAddr;
};

struct Image {
	VkImage vkImage;
	VkDeviceMemory vkMemory;
	VkDeviceAddress deviceAddr;
};

class Device
{
public:
	static void enableDeviceValidation();
	static VkDevice get();
	static void destroy();
	static VkQueue getGraphicQueue();
	static VkQueue getPresentQueue();
	static VkQueue getTransferQueue();
	static VkCommandPool getGraphicCmdPool();
	static VkCommandPool getTransferCmdPool();
	static void createCommandPool(int queueFamily, VkCommandPool* commandPool);
private:
	static void createDevice();
	static VkDevice device;
	static VkQueue graphicQueue;
	static VkQueue transferQueue;
	static VkQueue presentQueue;
	static VkCommandPool graphicCommandPool;
	static VkCommandPool transferCommandPool;
	static bool validation;
	static bool ready;
};

