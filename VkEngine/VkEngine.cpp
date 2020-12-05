#include "VkEngine.h"
#include "Scene3D.h"
#include "Instance.h"
#include "Debug.h"
#include "ApiUtils.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "Renderer.h"
#include "Pipeline.h"
#include "raytracing.h"
#include "commons.h"

namespace vkengine
{
	double unified_delta_time;
	SurfaceOwner* surfaceOwner;
	std::string active_scene;
	std::unordered_map<std::string, Scene3D>* scenes;

	void buildBasicPipelines();
	void recreateSwapChain();

	void setSurfaceOwner(SurfaceOwner * surface_owner)
	{
		surfaceOwner = surface_owner;
	}

	void init()
	{
#ifdef DEBUG
		Instance::enableValidation();
#endif
		Instance::setAppName("Demo");
		Instance::setEngineName("VkEngine");
		Instance::setSurfaceOwner(surfaceOwner);
		PhysicalDevice::setSurface(static_cast<VkSurfaceKHR>
			(surfaceOwner->getSurface(Instance::get())));
		PhysicalDevice::get();
		if (Instance::hasValidation()) Device::enableDeviceValidation();
		Device::get();
		if (hasRayTracing()) {
			RayTracer::initialize();
			Renderer::useRayTracing = true;
		}
		SwapChainMng::init(surfaceOwner);
		RenderPassCatalog::init();
		PipelineFactory::init();
		buildBasicPipelines();
		MeshManager::init();
		TextureManager::init();
		Renderer::init();
		scenes = new std::unordered_map<std::string, Scene3D>();
	}

	void resizeSwapchain()
	{
		recreateSwapChain();
	}

	void shutdown()
	{
		vkDeviceWaitIdle(Device::get());
		scenes->clear();
		delete scenes;
		RayTracer::cleanUP();
		PipelineFactory::cleanUP();
		DescriptorSetsFactory::cleanUp();
		Renderer::cleanUp();
		RenderPassCatalog::cleanUP();
		SwapChainMng::cleanUP();
		TextureManager::cleanUp();
		MeshManager::cleanUp();
		Device::destroy();
		Instance::destroyInstance();
	}

	void loadMesh(std::string id, std::string mesh_file)
	{
		MeshManager::addMesh(id, mesh_file);
	}

	std::vector<std::string> listLoadedMesh()
	{
		return MeshManager::listLoadedMeshes();
	}

	void loadTexture(std::string id, std::string texture_file)
	{
		TextureManager::addTexture(id, texture_file);
	}

	std::vector<std::string> listLoadedTextures()
	{
		return TextureManager::listSceneTextures();
	}

	void loadCubeMap(std::string id, std::string texture_file)
	{
		TextureManager::addCubeMap(id, texture_file);
	}

	std::vector<const char*> list_scenes()
	{
		std::vector<const char*> scene_ids;
		scene_ids.reserve(scenes->size());
		for (auto& entry : *scenes) {
			scene_ids.push_back(entry.first.c_str());
		}
		return scene_ids;
	}

	void createScene(std::string scene_id, std::string name)
	{
		scenes->insert({ scene_id, {scene_id, name} });
	}

	Scene3D* getActiveScene()
	{
		return &scenes->at(active_scene);
	}

	Scene3D* getScene(std::string scene_id)
	{
		return &scenes->at(scene_id);
	}

	void removeScene(std::string scene_id)
	{
		scenes->erase(scene_id);
		if (scenes->size() == 0) {
			createScene("scene","Scene");
		}
		loadScene((*scenes->begin()).first);
	}

	void loadFontAtlas(unsigned char * pixels, int * width, int * height)
	{
		TextureManager::loadFontAtlasTexture(pixels, width, height);
		PipelineFactory::updatePipelineResources(PIPELINE_LAYOUT_IMGUI);
	}

	void updateImGuiData(UiDrawData draw_data)
	{
		MeshManager::updateImGuiBuffers(draw_data,
			Renderer::getNextFrameBufferIndex());
	}

	void reloadScene()
	{
		return loadScene(active_scene);
	}

	void loadScene(std::string scene_id)
	{
		active_scene = scene_id;
		Renderer::prepareScene(&scenes->at(active_scene));
		// Just in case resources like Textures were added / updated
		PipelineFactory::updatePipelineResources(PIPELINE_LAYOUT_STANDARD);
		if (hasRayTracing()) {
			RayTracer::updateRTPipelineResources(&scenes->at(active_scene));
		}
	}

	bool* multithreadedRendering()
	{
		return &Renderer::multithreading;
	}

	bool hasRayTracing()
	{
		return PhysicalDevice::hasRaytracing();
	}

	bool * rayTracing()
	{
		return &Renderer::useRayTracing;
	}

	uint32_t* rayMaxDepth()
	{
		return &RayTracer::max_reflections_depth;
	}

	void renderFrame()
	{
		if (!Renderer::prepareFrame()) {
			recreateSwapChain();
			return;
		}
		Renderer::renderScene();

		if (!Renderer::finalizeFrame()) {
			recreateSwapChain();
			return;
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
		vkDeviceWaitIdle(Device::get());
		Renderer::cleanUp();
		RenderPassCatalog::cleanUP();
		SwapChainMng::cleanUP();

		for (auto & s : *scenes) {
			scenes->at(s.first).getCamera(scenes->at(s.first).current_camera)->updateAspectRatio(width, height);
		}

		SwapChainMng::init(surfaceOwner);
		RenderPassCatalog::init();
		PipelineFactory::updatePipelinesViewPorts();
		Renderer::init();
		Renderer::prepareScene(&scenes->at(active_scene));

		// IMGUI pipeline layout uses an offscreen attachment 
		// which is recreated by the Renderer to match the swapchain extent
		// For this reason i have to update his descriptors
		PipelineFactory::updatePipelineResources(PIPELINE_LAYOUT_IMGUI);		
		if (hasRayTracing()) {
			RayTracer::updateRTPipelineResources(&scenes->at(active_scene));
		}
	}

	void buildBasicPipelines() 
	{
		// Standard 3D rendering to offscreen target
		PipelineFactory::newPipeline(STD_3D_PIPELINE_ID, &RenderPassCatalog::offscreenRP,
			0, PipelineLayoutType::PIPELINE_LAYOUT_STANDARD);
		PipelineFactory::setShaders("VkEngine/Shaders/phong_multi_light/vert.spv", "VkEngine/Shaders/phong_multi_light/frag.spv");

		// Imgui rendering to final presentation on swapchain
		PipelineFactory::newPipeline(IMGUI_PIPELINE_ID, &RenderPassCatalog::presentationRP,
			0, PipelineLayoutType::PIPELINE_LAYOUT_IMGUI);
		PipelineFactory::setVertexType(VertexTypes::VERTEX_2D);
		PipelineFactory::setShaders("VkEngine/Shaders/imgui/vert.spv", "VkEngine/Shaders/imgui/frag.spv");
		PipelineFactory::setCulling(false);
		PipelineFactory::setDepthTest(false);
		PipelineFactory::setDynamicViewPortAndScissor();
		// Building
		PipelineFactory::createRasterizationPipelines();

		if (hasRayTracing()) {
			RayTracer::createRayTracingPipeline();
			RayTracer::createShaderBindingTable();
		}
	}
}
