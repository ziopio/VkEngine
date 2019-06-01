#include "stdafx.h"
#include "PhysicalDevice.h"
#include "Instance.h"


VkPhysicalDevice PhysicalDevice::physicalDevice = VK_NULL_HANDLE;
VkSurfaceKHR PhysicalDevice::surface = VK_NULL_HANDLE;
QueueFamilyIndices PhysicalDevice::queueFamilyIndices;
SwapChainSupportDetails PhysicalDevice::swapChainSupportDetails;
bool PhysicalDevice::ready;

void PhysicalDevice::setSurface(VkSurfaceKHR surface)
{
	if (ready) return;
	PhysicalDevice::surface = surface;
}

VkSurfaceKHR PhysicalDevice::getSurface()
{
	return surface;
}

VkPhysicalDevice PhysicalDevice::get()
{
	if (!ready) {
		pickPhysicalDevice();
		ready = true;
	}
	return PhysicalDevice::physicalDevice;
}

QueueFamilyIndices PhysicalDevice::getQueueFamilies()
{
	if (!ready) throw std::runtime_error("PhysicalDevice Not Ready!!");
	return PhysicalDevice::queueFamilyIndices;
}

SwapChainSupportDetails PhysicalDevice::getSwapChainSupport()
{
	if (!ready) throw std::runtime_error("PhysicalDevice Not Ready!!");
	return swapChainSupportDetails = PhysicalDevice::querySwapChainSupport(physicalDevice);
}

void PhysicalDevice::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Instance::get(), &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(Instance::get(), &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool PhysicalDevice::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// trovo una coda utilizzabile
	queueFamilyIndices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		swapChainSupportDetails = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
	}

	return deviceProperties.deviceType &
		(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU | VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) //dedicata o integrata 
		&& deviceFeatures.geometryShader && queueFamilyIndices.isComplete() // che supporti il geometry shader e abbia le code richieste
		&& extensionsSupported && swapChainAdequate // supporti le estensioni di superficie e supporti una swap_chain compatibile
		&& deviceFeatures.samplerAnisotropy; // supporti il multisampling
}

QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);


	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	// Voglio trovare una famiglia di code che soddisfi almeno VK_QUEUE_GRAPHICS_BIT
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// chiedo se il tipo di coda di indice i è compatilbile con la superficie
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}
		//NOTA: possono esserci code diverse per presentazione e per comandi di disegno!
		//NOTA: utilizzare la stessa coda per disegno e presentazione potrebbe ottimizzare le prestazioni

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			indices.transferFamily = i;
		}

		if (indices.isComplete()) {
			break; // esco appena trovo l'indice della prima coda che mi interessa
		}

		i++;
	}
	return indices;
}

bool PhysicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	std::cout << extensionCount << " available extensions for the GPU:" << std::endl;

	for (const auto& extension : availableExtensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty(); // se è vuoto allora ho trovato tutte le estensioni
}

SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	// Richiedo le capacità del dispositivo di interagire con la superficie
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Richiedo i formati supportati dal dispositivo e dalla superficie
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	//Richiedo le modalità di presentazione disponibili tra dispositivo e superficie
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}
