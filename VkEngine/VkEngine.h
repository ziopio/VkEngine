#pragma once
#include <vector>
#include "MessageManager.h"

typedef struct{
	unsigned int instance_extension_count;
	const char** instanceExtensions;

} VkEngineInitInfo;

class Object;
class LightSource;
class Surface;
class SwapChain;
class RenderPass;
class Renderer;

class VkEngine : MsgReceiver
{
public:
	VkEngine();
	void initialize(VkEngineInitInfo info);
	void loadMesh(std::string mesh_file);
	void loadTexture(std::string texture_file);
	void setLights(std::vector<LightSource*> lights);
	void setObjects(std::vector<Object*> objects);
	void renderFrame();
	void cleanUp();
	~VkEngine();
	bool validation;
private:
	void initVulkan();
	void recreateSwapChain();
	void drawFrame();
	void cleanupSwapChain();
	void receiveMessage(Message msg);
	MessageManager* msgManager;
	std::vector<Object*> objects;
	std::vector<LightSource*> lights;
	Surface* surface;
	SwapChain* swapChain;
	RenderPass* renderPass;
	Renderer* renderer;
	int fps;
	bool terminate;
};

