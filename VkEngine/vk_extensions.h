#pragma once
#define VK_ENABLE_BETA_EXTENSIONS // Needed for Nvidia RTX
#include <vulkan/vulkan.h>


// VK_KHR_ray_tracing
static const char* rayTracingDeviceExtensions[] = {
   VK_KHR_RAY_TRACING_EXTENSION_NAME,
   VK_KHR_MAINTENANCE3_EXTENSION_NAME,
   VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
   VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
   VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
   VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
};

// Loads the function pointers required for ray tracing
void LOAD_RAYTRACING_API_COMMANDS(VkDevice device);