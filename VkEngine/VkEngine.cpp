#include "stdafx.h"
#include "VkEngine.h"
#include "Instance.h"
#include "Debug.h"
#include "ApiUtils.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "DescriptorSetsFactory.h"
#include "Object.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Renderer.h"
#include "Direction.h"
#include "LightSource.h"

VkEngine::VkEngine()
{
	msgManager.registerListener(this);
}

void VkEngine::setSurfaceOwner(SurfaceOwner * surfaceOwner)
{
	this->surfaceOwner = surfaceOwner;
}

void VkEngine::init()
{
	Instance::setAppName("Demo");
	Instance::setEngineName("VkEngine");
	Instance::setRequiredExtensions(surfaceOwner->getInstanceExtInfo());
	PhysicalDevice::setSurface(static_cast<VkSurfaceKHR>
		(surfaceOwner->getSurface(Instance::get())));
	PhysicalDevice::get();
	if (Instance::hasValidation()) Device::enableDeviceValidation();
	Device::get();
	swapChain = new SwapChain(this->surfaceOwner);
	renderPass = new RenderPass(swapChain);
	MaterialManager::init(swapChain, renderPass);
	MeshManager::init();
	TextureManager::init();
	DescriptorSetsFactory::init(swapChain);
	int width, height;
	surfaceOwner->getFrameBufferSize(&width,&height);
	Direction::updateCamerasScreenSize(width, height);
	Direction::initialize();
	renderer = new Renderer(renderPass, swapChain);
}

void VkEngine::resizeSwapchain(int width, int height)
{
	this->recreateSwapChain();
}

Camera * VkEngine::getCurrentCamera()
{
	return Direction::getCurrentCamera();
}

VkEngine::~VkEngine()
{
	Direction::cleanUp();
	cleanupSwapChain();
	DescriptorSetsFactory::cleanUp();
	TextureManager::cleanUp();
	MeshManager::cleanUp();
	Device::destroy();
	Instance::destroyInstance();
}

void VkEngine::loadMesh(std::string mesh_file)
{
	MeshManager::addMesh(mesh_file);
}

void VkEngine::loadTexture(std::string texture_file)
{
	DescriptorSetsFactory::cleanUp();
	TextureManager::addTexture(texture_file);
	DescriptorSetsFactory::init(this->swapChain);
}

void VkEngine::addLight(PointLightInfo light_info)
{
	LightSource light(glm::make_vec3(light_info.position), glm::make_vec3(light_info.color),light_info.power);
	if (lights.size() < 10)
		this->lights.push_back(light);
	renderer->setLights(lights);
}

void VkEngine::addObject(ObjectInitInfo _obj)
{
	Object obj(_obj.mesh_id,(MaterialType)_obj.material_id,
		_obj.texture_id,_obj.transformation);
	this->objects.push_back(obj);
	renderer->setObjects(objects);
}

void VkEngine::renderFrame()
{
	//InputControl::processInput();
	this->msgManager.dispatchMessages();	
	if (!renderer->renderScene()) {
		this->recreateSwapChain();
	}
	//vkDeviceWaitIdle(Device::get());
}

void VkEngine::recreateSwapChain() { 
	// TODO: comporre la nuova swapchain riagganciando la vecchia
	int width = 0, height = 0;
	this->surfaceOwner->getFrameBufferSize(&width, &height);
	while (width == 0 || height == 0) {
		this->surfaceOwner->waitEvents();// window is minimized so application stops
		this->surfaceOwner->getFrameBufferSize(&width, &height);
	}
	cleanupSwapChain();

	Direction::updateCamerasScreenSize(width, height);
	swapChain = new SwapChain(this->surfaceOwner);
	renderPass = new RenderPass(swapChain);
	MaterialManager::init(swapChain, renderPass);
	renderer = new Renderer(renderPass, swapChain);
	renderer->setObjects(this->objects);
	renderer->setLights(this->lights);
}


void VkEngine::cleanupSwapChain()
{
	vkDeviceWaitIdle(Device::get());
	delete renderer;
	MaterialManager::destroyAllMaterials();
	delete renderPass;
	delete swapChain;
}

void VkEngine::receiveMessage(Message msg)
{
	switch (msg) {
	case Message::MULTITHREADED_RENDERING_ON_OFF: 
		if (renderer->multithreading) {
			renderer->multithreading = false;
			printf("\nCommandBuffers generation ON MAIN THREAD");
		}
		else {
			renderer->multithreading = true;
			printf("\nCommandBuffers generation ON ALL AVAILABLE THREADS");
		}			
		break;
	default:
		break;
	}
}