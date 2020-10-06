#pragma once
#include "commons.h"


VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

void copyBufferToBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void createImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height,
	VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

bool hasStencilComponent(VkFormat format);

void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

VkCommandBuffer beginSingleTimeCommandBuffer(VkDevice device, VkCommandPool commandPool);

void submitAndWaitCommandBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer & commandBuffer);


void submitAndWaitCommandBuffers(VkDevice device, VkQueue queue, VkCommandPool commandPool, std::vector<VkCommandBuffer> & commandBuffers);
