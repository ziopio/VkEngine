#include "stdafx.h"
#include "Instance.h"
#include "Debug.h"

std::string  Instance::appName = "...";
std::string Instance::EngineName = "...";
const char ** Instance::surfaceExtensions;
unsigned int Instance::surfaceExtCount;
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
	std::vector<const char*> glfwExtensions = Instance::getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();
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
	std::vector<VkExtensionProperties> ALLextensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, ALLextensions.data());

	std::cout << extensionCount << " available extensions for vulkan:" << std::endl;
	for (const auto& extension : ALLextensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	// Cerco tutte le estensioni richieste da GLFW
	auto info = Instance::surfaceOwner->getInstanceExtInfo();
	Instance::surfaceExtensions = info.instanceExtensions;
	Instance::surfaceExtCount = info.instance_extension_count;
	const char** surfaceRequiredExtensions = surfaceExtensions;
	std::cout << surfaceExtCount << " required vulkan extensions for GLFW:" << std::endl;
	std::vector<const char*> extensions(surfaceRequiredExtensions, surfaceRequiredExtensions + surfaceExtCount);
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension << std::endl;
	}

	if (validation) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
