#include "Device.h"
#include "PhysicalDevice.h"
#include "Debug.h"
#include "commons.h"

VkDevice Device::device = VK_NULL_HANDLE;
VkQueue Device::graphicQueue = VK_NULL_HANDLE;
VkQueue Device::presentQueue = VK_NULL_HANDLE;
VkQueue Device::transferQueue = VK_NULL_HANDLE;
VkCommandPool Device::graphicCommandPool = VK_NULL_HANDLE;
VkCommandPool Device::transferCommandPool = VK_NULL_HANDLE;

bool Device::validation;
bool Device::ready;

void Device::enableDeviceValidation()
{
	validation = true;
}


VkDevice Device::get()
{
	if (!ready) {
		createDevice();
		createCommandPool(PhysicalDevice::getQueueFamilies().graphicsFamily, &graphicCommandPool);
		createCommandPool(PhysicalDevice::getQueueFamilies().transferFamily, &transferCommandPool);
		ready = true;
	}
	return Device::device;
}

void Device::destroy()
{
	if(!ready)  throw std::runtime_error("Device Not Ready!!");
	vkDestroyCommandPool(Device::get(), Device::getGraphicCmdPool(), nullptr);
	vkDestroyCommandPool(Device::get(), Device::getTransferCmdPool(), nullptr);
	vkDestroyDevice(Device::get(), nullptr);
	ready = false;
}

VkQueue Device::getGraphicQueue()
{
	if (!ready) throw std::runtime_error("Device Not Ready!!");
	return Device::graphicQueue;
}

VkQueue Device::getPresentQueue()
{
	if (!ready) throw std::runtime_error("Device Not Ready!!");
	return Device::presentQueue;
}

VkQueue Device::getTransferQueue()
{
	if (!ready) throw std::runtime_error("Device Not Ready!!");
	return Device::transferQueue;
}

VkCommandPool Device::getGraphicCmdPool()
{
	if (!ready) throw std::runtime_error("Device Not Ready!!");
	return Device::graphicCommandPool;
}

VkCommandPool Device::getTransferCmdPool()
{
	if (!ready) throw std::runtime_error("Device Not Ready!!");
	return Device::transferCommandPool;
}

void Device::createDevice() {
	// Prendo l'indice della coda selezionata durante la scelta del dispositivo fisico
	QueueFamilyIndices indices = PhysicalDevice::getQueueFamilies();
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
	//NOTA: usare un set accorpa gli indici uguali, quindi se le due code sono una sola il set le accorpa.

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		// Bisogna specificare la priorità della coda tra 0.0 e 1.0
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data(); // aggancio l'array di info delle code
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	// We use VkPhysicalDeviceFeatures2 so all features are taken from pNext
	createInfo.pEnabledFeatures = nullptr; 
	createInfo.pNext = &PhysicalDevice::getPhysicalDeviceFeatures();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (validation) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	// creo il logicDevice indicando il phisicalDevice e le info di creazione
	if (vkCreateDevice(PhysicalDevice::get(), &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	// recupero la coda del logic device appena creato
	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicQueue); // indice 0 siccome ho una sola coda
	vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue); // indice 0 siccome ho una sola coda
	vkGetDeviceQueue(device, indices.transferFamily, 0, &transferQueue); // indice 0 siccome ho una sola coda

}

void Device::createCommandPool(int queueFamily, VkCommandPool* commandPool)
{
	VkCommandPoolCreateInfo pool_Info = {};
	pool_Info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_Info.queueFamilyIndex = queueFamily;
	pool_Info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffer reset

	if (vkCreateCommandPool(device, &pool_Info, nullptr, commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphic command pool!");
	}
}
