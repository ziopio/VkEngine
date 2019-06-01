#pragma once
#include "Mesh.h"

class MeshManager
{
public:
	static void init();
	static void addMesh(std::string mesh_path);
	static Mesh* getMesh(int id);
	static void cleanUp();
private:
	static std::vector<Mesh*> meshes;
};

