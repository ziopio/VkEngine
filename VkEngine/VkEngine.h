#pragma once
#include <vector>
#include "MessageManager.h"

typedef struct {
	unsigned int instance_extension_count;
	const char** instanceExtensions;
	bool enableValidation;
} VulkanInstanceInitInfo;

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
	void* createInstance(VulkanInstanceInitInfo info);
	void setSurface(void * surface);
	void init();
	void loadMesh(std::string mesh_file);
	void loadTexture(std::string texture_file);
	void setLights(std::vector<LightSource*> lights);
	void setObjects(std::vector<Object*> objects);
	void renderFrame();
	~VkEngine();
private:
	void recreateSwapChain();
	void drawFrame();
	void cleanupSwapChain();
	void receiveMessage(Message msg);
	MessageManager* msgManager;
	std::vector<Object*> objects;
	std::vector<LightSource*> lights;
	SwapChain* swapChain;
	RenderPass* renderPass;
	Renderer* renderer;
	int fps;
	bool terminate;
};

