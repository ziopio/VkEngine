#pragma once
#include "Mesh.h"

using namespace vkengine;

class MeshManager
{
public:
	static void init();
	static void addMesh(std::string id, std::string mesh_path);
	static BaseMesh* getMesh(std::string id);
	static std::vector<std::string> listLoadedMeshes();
	static GuiMesh* getImGuiMesh(unsigned imageIndex);
	static void updateImGuiBuffers(UiDrawData imgui, unsigned imageIndex);
	static void cleanUp();
private:
	static std::unordered_map<std::string, Mesh3D*> scene_meshes;
	// one mesh and buffers for each frame to be able to update the mesh 
	// for one frame while rendering on the others.
	static std::vector<GuiMesh*> per_frame_imguis;
};

