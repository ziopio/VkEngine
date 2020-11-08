#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Scene3D.h"


#define OFFSCREEN_FRAMEBUFFER_TEXTURE_ID -1 // special case in gui fragment shader

namespace vkengine
{
	// Global delta time used inside the engine in each iteration
	extern double unified_delta_time;

	typedef struct {
		unsigned int instance_extension_count;
		const char** instanceExtensions;
	} VulkanInstanceInitInfo;

	// This structs serve to remove dependencies from imgui types
	typedef struct {
		uint32_t elementCount;
		glm::vec4 clipRectangle;
		uint32_t textureID;
	} UiDrawCmd;

	typedef struct {
		std::vector<UiDrawCmd> drawCommands;
		void* vertexBuffer;
		int vertexBufferSize;
		void* indexBuffer;
		int indexBufferSize;
	} UiDrawList;

	struct UiDrawData {
		glm::vec2 frame_buffer_scale;
		glm::vec2 display_pos;
		glm::vec2 display_size;
		size_t totalVtxCount;
		size_t totalIdxCount;
		std::vector<UiDrawList> drawLists;
	};

	class SurfaceOwner {
	protected:
		void* surface = nullptr;
	public:
		virtual VulkanInstanceInitInfo getInstanceExtInfo() = 0;
		virtual void* getSurface(void* vulkan_instance) = 0;
		virtual void getFrameBufferSize(int* width, int* height) = 0;
		virtual void printDebug(std::string msg) = 0;
		virtual void waitEvents() = 0;
	};

	void setSurfaceOwner(SurfaceOwner* surface_owner);
	void init();
	void resizeSwapchain();

	void loadMesh(std::string id, std::string mesh_file);
	std::vector<std::string> listLoadedMesh();
	void loadTexture(std::string id, std::string texture_file);
	std::vector<std::string> listLoadedTextures();


	void createScene(std::string scene_id, std::string name );
	Scene3D* getScene(std::string scene_id);
	void removeScene(std::string scene_id);

	void loadFontAtlas(unsigned char* pixels, int * width, int * height);
	void updateImGuiData(UiDrawData draw_data);
	// to be called every time an element in the scene is added or removed
	void loadScene(std::string scene_id);
	// Intended as parallel CMD buffer recording CPU-side
	bool* multithreadedRendering();
	bool hasRayTracing();
	bool* rayTracing();

	void renderFrame();
	void shutdown();

}

