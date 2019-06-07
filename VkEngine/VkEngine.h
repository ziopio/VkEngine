#pragma once
#include <vector>
#include "MessageManager.h"


class Object;
class LightSource;
class Surface;
class SwapChain;
class RenderPass;
class Renderer;

typedef struct {
	unsigned int instance_extension_count;
	const char** instanceExtensions;
	bool enableValidation;
} VulkanInstanceInitInfo;

typedef struct {
	float position[3];
	float color[3];
	float power;
} PointLightInfo;
typedef struct {
	float position[3];
	/* Axis of the rotation	*/
	float rotation_vector[3];
	float angularSpeed;
	float scale_vector[3];
	float scale_factor;
}ObjTransformation;
typedef struct {
	int mesh_id;
	int material_id;
	int texture_id;
	ObjTransformation transformation;
}ObjectInitInfo;

class SurfaceOwner {
public:
	virtual VulkanInstanceInitInfo getInstanceExtInfo() = 0;
	virtual void* getSurface(void* vulkan_instance) = 0;
	virtual void getFrameBufferSize(int* width, int* height) = 0;
	virtual void waitEvents() = 0;
};

class VkEngine : MsgReceiver
{
public:
	VkEngine();
	void setSurfaceOwner(SurfaceOwner* surface_owner);
	void init();
	void resizeSwapchain(int width, int height);
	void loadMesh(std::string mesh_file);
	void loadTexture(std::string texture_file);
	void addLight(PointLightInfo light);
	void addObject(ObjectInitInfo obj_info);
	void renderFrame();
	~VkEngine();
private:
	void recreateSwapChain();
	void cleanupSwapChain();
	void receiveMessage(Message msg);
	SurfaceOwner* surfaceOwner;
	MessageManager msgManager;
	std::vector<Object> objects;
	std::vector<LightSource> lights;
	SwapChain* swapChain;
	RenderPass* renderPass;
	Renderer* renderer;
};
