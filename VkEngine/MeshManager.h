#pragma once
#include "Mesh.h"

struct UiDrawData;

class MeshManager
{
public:
	static void init();
	static void addMesh(std::string mesh_path);
	static Mesh* getMesh(int id);
	static void updateImGuiBuffers(UiDrawData imgui);
	static void cleanUp();
private:
	static std::vector<Mesh*> scene_meshes;
	static Mesh* imgui;
};

