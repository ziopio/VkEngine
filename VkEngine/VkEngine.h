#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Object.h"
#include "Camera.h"
#include "LightSource.h"

namespace vkengine
{
	typedef struct {
		unsigned int instance_extension_count;
		const char** instanceExtensions;
		bool enableValidation;
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
	void loadTexture(std::string id, std::string texture_file);

	void createScene(const char* scene_id);

	void loadFontAtlas(unsigned char* pixels, int * width, int * height);
	void updateImGuiData(UiDrawData draw_data);
	void addLight(PointLightInfo light);
	void addObject(ObjectInitInfo obj_info);
	void renderFrame();
	void shutdown();

}

