#pragma once
#include "VkEngine.h"
#include "commons.h"

using namespace vkengine;

class Instance
{
public:
	static void setAppName(std::string appName);
	static void setEngineName(std::string engineName);
	static void setSurfaceOwner( SurfaceOwner* surfaceOwner);
	static VkInstance get();
	static void enableValidation();
	static bool hasValidation();
	static void destroyInstance();
private:
	static void createInstance();
	static bool checkValidationLayerSupport();
	static std::vector<const char*> getRequiredExtensions();
	static std::string appName;
	static std::string EngineName;
	static const char ** surfaceExtensions;
	static unsigned int surfaceExtCount;
	static SurfaceOwner* surfaceOwner;
	static VkInstance instance;
	static VkDebugUtilsMessengerEXT messangerExtension;
	static bool ready;
	static bool validation;
};

