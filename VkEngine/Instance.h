#pragma once

class Instance
{
public:
	static void setAppName(std::string appName);
	static void setEngineName(std::string engineName);
	static void enableValidationLayers();
	static VkInstance get();
	static void destroyInstance();
private:
	static void createInstance();
	static bool checkValidationLayerSupport();
	static std::vector<const char*> getRequiredExtensions();
	static std::string appName;
	static std::string EngineName;
	static VkInstance instance;
	static VkDebugUtilsMessengerEXT messangerExtension;
	static bool validation;
	static bool ready;
};

