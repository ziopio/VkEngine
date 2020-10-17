#pragma once
#include "Mesh.h"

class MeshManager
{
public:
	static void init();
	static void addMesh(std::string id, std::string mesh_path);
	static Mesh3D* getMesh(unsigned id);
	static Mesh3D* getMesh(std::string string_id);
	static unsigned getMeshID(std::string string_id);
	static std::vector<Mesh3D*> getMeshLibrary();
	static std::vector<std::string> listLoadedMeshes();
	static GuiMesh* getImGuiMesh(unsigned imageIndex);
	static void updateImGuiBuffers(vkengine::UiDrawData imgui, unsigned imageIndex);
	static void cleanUp();
private:
	static std::unordered_map<std::string, unsigned> mesh_ids;
	static std::vector<Mesh3D*> mesh_library;
	// one mesh and buffers for each frame to be able to update the mesh 
	// for one frame while rendering on the others.
	static std::vector<GuiMesh*> per_frame_imguis;
};

