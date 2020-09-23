#include "Instance.h"
#include "Debug.h"
#include "raytracing.h"
#include "commons.h"

const std::vector<const char*> advancedInstanceExtensions = {
	VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
};


std::string  Instance::appName = "...";
std::string Instance::EngineName = "...";
//const char ** Instance::surfaceExtensions;
//unsigned int Instance::surfaceExtCount;
VkInstance Instance::instance;
SurfaceOwner* Instance::surfaceOwner;
VkDebugUtilsMessengerEXT Instance::messangerExtension;
bool Instance::validation;
bool Instance::ready;

void Instance::setAppName(std::string appName)
{
	if (ready) return;
	Instance::appName = appName;
}

void Instance::setEngineName(std::string engineName)
{
	if (ready) return;
	Instance::EngineName = engineName;
}

void Instance::setSurfaceOwner(SurfaceOwner* surfaceOwner)
{
	Instance::surfaceOwner = surfaceOwner;
}

VkInstance Instance::get()
{
	if (!ready) {
		createInstance();
		ready = true;
		if (validation) {
			setupDebugCallback(instance, &messangerExtension,(void*)Instance::surfaceOwner);
		}
	}
	return instance;
}

void Instance::enableValidation()
{
	validation = true;
}

bool Instance::hasValidation()
{
	return validation;
}

void Instance::destroyInstance()
{
	if(!ready) throw std::runtime_error("Instance Not Ready!!");
	if (validation) {
		destroyDebugUtilsMessengerEXT(instance, messangerExtension, nullptr);
	}	
	vkDestroySurfaceKHR(Instance::get(), (VkSurfaceKHR)Instance::surfaceOwner->getSurface(instance), nullptr);
	vkDestroyInstance(instance, nullptr);
	ready = false;
}

void Instance::createInstance()
{
	// raccolgo info sulla mia applicazione
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = EngineName.c_str();
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	// creo la struttura di info per la creazione dell'istanza di Vulkan
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Chiedo a glfw di cercare tutte le estensioni richieste per GLFW
	std::vector<const char*> requiredExtensions = Instance::getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	//Controllo a presenza dei validationLayers
	if (validation ) {
		if (!checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	//Creazione dell'istanza di Vulkan
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

bool Instance::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> Instance::getRequiredExtensions() {

	// Cerco tutte le estensioni supportate ( NON NECESSARIO )
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::cout << extensionCount << " available extensions for vulkan:" << std::endl;
	for (const auto& extension : availableExtensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	// Cerco tutte le estensioni richieste da GLFW
	auto surface = Instance::surfaceOwner->getInstanceExtInfo();
	std::vector<const char*> requiredExtensions(surface.instanceExtensions, surface.instanceExtensions + surface.instance_extension_count);
	requiredExtensions.insert(requiredExtensions.end(), advancedInstanceExtensions.begin(), advancedInstanceExtensions.end());
	if (validation) {
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	std::cout << requiredExtensions.size() << " required extensions for the engine:" << std::endl;
	for (const auto& extension : requiredExtensions) {
		std::cout << "\t" << extension << std::endl;
	}

	return requiredExtensions;
}
