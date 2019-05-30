#include "stdafx.h"
#include "MeshManager.h"

std::vector<Mesh*> MeshManager::meshes;


void MeshManager::addMesh(std::string mesh_path)
{
	meshes.push_back(new Mesh(mesh_path));
}

Mesh * MeshManager::getMesh(int id)
{
	return meshes[id];
}

void MeshManager::cleanUp()
{
	for (auto mesh : meshes) {
		delete mesh;
	}
}
