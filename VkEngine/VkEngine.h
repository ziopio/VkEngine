#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace vkengine
{

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

	// This structs serve to remove dependencies from imgui types
	typedef struct {
		uint32_t elementCount;
		glm::vec4 clipRectangle;
		uint32_t textureID;
	}UiDrawCmd;

	typedef struct {
		std::vector<UiDrawCmd> drawCommands;
		void* vertexBuffer;
		int vertexBufferSize;
		void* indexBuffer;
		int indexBufferSize;
	}UiDrawList;
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

	
	void loadScene(const char* scene_file);
	void loadMesh(std::string mesh_file);
	void loadTexture(std::string texture_file);
	void loadFontAtlas(unsigned char* pixels, int * width, int * height);
	void updateImGuiData(UiDrawData draw_data);
	void addLight(PointLightInfo light);
	void addObject(ObjectInitInfo obj_info);
	void renderFrame();
	void shutdown();

}

