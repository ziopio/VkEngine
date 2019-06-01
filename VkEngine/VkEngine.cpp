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
	this->msgManager = new MessageManager();
	msgManager->registerListener(this);
}

void* VkEngine::createInstance(VulkanInstanceInitInfo info)
{
	//InputControl::init(msgManager, window);
	//Direction::initialize(HEIGHT, WIDTH);
	//Direction::addCamera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	Instance::setAppName("Demo");
	Instance::setEngineName("VkEngine");
	Instance::setRequiredExtensions(info);
	return Instance::get();
}

void VkEngine::setSurface(void * surface)
{
	PhysicalDevice::setSurface(static_cast<VkSurfaceKHR>(surface));
}

void VkEngine::init()
{
	PhysicalDevice::get();
	if (Instance::hasValidation()) Device::enableDeviceValidation();
	Device::get();
	swapChain = new SwapChain();
	renderPass = new RenderPass(swapChain);
	MaterialManager::init(swapChain, renderPass);
	MeshManager::init();
	TextureManager::init();
	DescriptorSetsFactory::init(swapChain,objects);
	renderer = new Renderer(renderPass, swapChain);

}

VkEngine::~VkEngine()
{
	Direction::cleanUp();
	cleanupSwapChain();
	DescriptorSetsFactory::cleanUp();
	TextureManager::cleanUp();
	MeshManager::cleanUp();
	for (auto obj : objects) {
		delete obj;
	}
	Device::destroy();
	Instance::destroyInstance();
}

void VkEngine::loadMesh(std::string mesh_file)
{
	MeshManager::addMesh(mesh_file);
}

void VkEngine::loadTexture(std::string texture_file)
{
	TextureManager::addTexture(texture_file);
}

void VkEngine::setLights(std::vector<LightSource*> lights)
{
	this->lights = lights;
	if (lights.size() > 10) this->lights.resize(10);
	renderer->setLights(lights);
}

void VkEngine::setObjects(std::vector<Object*> objects)
{
	this->objects = objects;

	DescriptorSetsFactory::init(swapChain, objects);

	renderer->setObjects(objects);
}

void VkEngine::renderFrame()
{
	//InputControl::processInput();
	this->msgManager->dispatchMessages();

	drawFrame();
}

void VkEngine::recreateSwapChain() { // TODO: comporre la nuova swapchain riagganciando la vecchia
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		//glfwWaitEvents(); // window is minimized so application stops
		//glfwGetFramebufferSize(window, &width, &height);
	}
	vkDeviceWaitIdle(Device::get());
	cleanupSwapChain();

	Direction::updateScreenSizes();
	swapChain = new SwapChain();
	renderPass = new RenderPass(swapChain);
	MaterialManager::init(swapChain, renderPass);
	renderer = new Renderer(renderPass, swapChain);
	renderer->setObjects(this->objects);
	renderer->setLights(lights);
}


void VkEngine::cleanupSwapChain()
{

	delete renderer;

	MaterialManager::destroyAllMaterials();

	delete renderPass;

	delete swapChain;
}

void VkEngine::drawFrame()
{
	if (!renderer->renderScene()) {
		this->recreateSwapChain();
	}
}

void VkEngine::receiveMessage(Message msg)
{
	switch (msg) {
	case Message::SHUT_DOWN: this->terminate = true;
		break;	
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