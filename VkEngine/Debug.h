#pragma once

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

// Funzione eseguita dalla callback
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunction(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

//Funzione "proxy" che carica la funzione "vkCreateDebugUtilsMessengerEXT" per una istanza vulkan
VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);

//Funzione "proxy" carica "vkDestroyDebugUtilsMessengerEXT" che distrugge l'estensione 
void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);

void setupDebugCallback(VkInstance instance, VkDebugUtilsMessengerEXT* callback);


