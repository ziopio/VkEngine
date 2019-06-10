#include "stdafx.h"
#include "MeshManager.h"
#include "VkEngine.h"

std::vector<Mesh*> MeshManager::scene_meshes;


void MeshManager::init()
{
	MeshManager::addMesh("VkEngine/Meshes/cube.obj");
}

void MeshManager::addMesh(std::string mesh_path)
{
	scene_meshes.push_back(new Mesh(mesh_path));
}

Mesh * MeshManager::getMesh(int id)
{
	return scene_meshes[id];
}

void MeshManager::updateImGuiBuffers(UiDrawData imgui)
{
}

void MeshManager::cleanUp()
{
	for (auto mesh : scene_meshes) {
		delete mesh;
	}
}
