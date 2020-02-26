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


namespace vkengine
{

	SurfaceOwner* surfaceOwner;
	std::vector<Object> objects;
	std::vector<LightSource> lights;
	SwapChain* swapChain;
	RenderPass* renderPass;
	Renderer* renderer;

	void recreateSwapChain();
	void cleanupSwapChain();

	void setSurfaceOwner(SurfaceOwner * surface_owner)
	{
		surfaceOwner = surface_owner;
	}

	void init()
	{
		Instance::setAppName("Demo");
		Instance::setEngineName("VkEngine");
		Instance::setSurfaceOwner(surfaceOwner);
		PhysicalDevice::setSurface(static_cast<VkSurfaceKHR>
			(surfaceOwner->getSurface(Instance::get())));
		PhysicalDevice::get();
		if (Instance::hasValidation()) Device::enableDeviceValidation();
		Device::get();
		swapChain = new SwapChain(surfaceOwner);
		renderPass = new RenderPass(swapChain);
		MaterialManager::init(swapChain, renderPass);
		MeshManager::init(swapChain->getImageViews().size());
		TextureManager::init();
		int width, height;
		surfaceOwner->getFrameBufferSize(&width, &height);
		Direction::updateCamerasScreenSize(width, height);
		Direction::initialize();
		renderer = new Renderer(renderPass, swapChain);
		DescriptorSetsFactory::init(swapChain, renderer);
	}

	void resizeSwapchain()
	{
		recreateSwapChain();
	}

	Camera * getCurrentCamera()
	{
		return Direction::getCurrentCamera();
	}

	void shutdown()
	{
		Direction::cleanUp();
		cleanupSwapChain();
		TextureManager::cleanUp();
		MeshManager::cleanUp();
		Device::destroy();
		Instance::destroyInstance();
	}

	void loadMesh(std::string id, std::string mesh_file)
	{
		MeshManager::addMesh(id, mesh_file);
	}

	void loadTexture(std::string id, std::string texture_file)
	{
		DescriptorSetsFactory::cleanUp();
		TextureManager::addTexture(id, texture_file);
		DescriptorSetsFactory::init(swapChain, renderer);
	}

	void loadFontAtlas(unsigned char * pixels, int * width, int * height)
	{
		TextureManager::loadFontAtlasTexture(pixels, width, height);
	}

	void updateImGuiData(UiDrawData draw_data)
	{
		MeshManager::updateImGuiBuffers(draw_data,
			renderer->getNextFrameBufferIndex());
	}

	void addLight(PointLightInfo light_info)
	{
		LightSource light(glm::make_vec3(light_info.position), glm::make_vec3(light_info.color), light_info.power);
		if (lights.size() < 10)
			lights.push_back(light);
		renderer->setLights(lights);
	}

	void addObject(ObjectInitInfo _obj)
	{
		Object obj(_obj.mesh_id, (MaterialType)_obj.material_id,
			_obj.texture_id, _obj.transformation);
		objects.push_back(obj);
		renderer->setObjects(objects);
	}

	void renderFrame()
	{
		//InputControl::processInput();
		//msgManager.dispatchMessages();	
		if (!renderer->renderScene()) {
			recreateSwapChain();
		}
		//vkDeviceWaitIdle(Device::get());
	}

	void recreateSwapChain() {
		// TODO: comporre la nuova swapchain riagganciando la vecchia
		int width = 0, height = 0;
		surfaceOwner->getFrameBufferSize(&width, &height);
		while (width == 0 || height == 0) {
			surfaceOwner->waitEvents();// window is minimized so application stops
			surfaceOwner->getFrameBufferSize(&width, &height);
		}
		cleanupSwapChain();

		Direction::updateCamerasScreenSize(width, height);
		swapChain = new SwapChain(surfaceOwner);
		renderPass = new RenderPass(swapChain);
		MaterialManager::init(swapChain, renderPass);
		renderer = new Renderer(renderPass, swapChain);
		renderer->setObjects(objects);
		renderer->setLights(lights);
		DescriptorSetsFactory::init(swapChain, renderer);
	}


	void cleanupSwapChain()
	{
		vkDeviceWaitIdle(Device::get());
		DescriptorSetsFactory::cleanUp();
		delete renderer;
		MaterialManager::destroyAllMaterials();
		delete renderPass;
		delete swapChain;
	}

}
