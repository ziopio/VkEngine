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
#include "Scene3D.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Renderer.h"


namespace vkengine
{

	SurfaceOwner* surfaceOwner;
	SwapChain* swapChain;
	RenderPass* renderPass;
	Renderer* renderer;
	std::string active_scene;
	std::unordered_map<std::string, Scene3D> scenes;

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
		//Direction::updateCamerasScreenSize(width, height);
		//Direction::initialize();
		renderer = new Renderer(renderPass, swapChain);
		DescriptorSetsFactory::init(swapChain, renderer);
	}

	void resizeSwapchain()
	{
		recreateSwapChain();
	}

	void shutdown()
	{
		//Direction::cleanUp();
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

	void createScene(std::string scene_id)
	{
		scenes.insert( { scene_id, Scene3D(scene_id) } );
	}

	Scene3D& getScene(std::string scene_id)
	{
		return scenes.at(scene_id);
	}

	void removeScene(std::string scene_id)
	{
		scenes.erase(scene_id);
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

	void loadScene(std::string scene_id)
	{
		active_scene = scene_id;
		renderer->prepareScene(&scenes.at(active_scene));
	}

	void renderFrame()
	{
		if (!renderer->renderScene()) {
			recreateSwapChain();
		}
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

		for (auto s : scenes) {
			scenes.at(s.first).getCurrentCamera()->updateAspectRatio(width, height);
		}

		swapChain = new SwapChain(surfaceOwner);
		renderPass = new RenderPass(swapChain);
		MaterialManager::init(swapChain, renderPass);
		renderer = new Renderer(renderPass, swapChain);
		renderer->prepareScene(&scenes.at(active_scene));
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
