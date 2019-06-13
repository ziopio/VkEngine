#pragma once
#include "Mesh.h"

struct UiDrawData;

class MeshManager
{
public:
	static void init(unsigned swapchain_image_count);
	static void addMesh(std::string mesh_path);
	static BaseMesh* getMesh(int id);
	static GuiMesh* getImGuiMesh(unsigned imageIndex);
	static void updateImGuiBuffers(UiDrawData imgui, unsigned imageIndex);
	static void cleanUp();
private:
	static std::vector<Mesh*> scene_meshes;
	// one mesh and buffers for each frame to be able to update the mesh 
	//for one frame while rendering on the others.
	static std::vector<GuiMesh*> per_frame_imguis;
};

