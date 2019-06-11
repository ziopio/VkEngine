#include "stdafx.h"
#include "MeshManager.h"
#include "VkEngine.h"

std::vector<Mesh*> MeshManager::scene_meshes;
GuiMesh* MeshManager::imgui;

void MeshManager::init()
{
	imgui = new GuiMesh();
	MeshManager::addMesh("VkEngine/Meshes/cube.obj");
}

void MeshManager::addMesh(std::string mesh_path)
{
	scene_meshes.push_back(new Mesh(mesh_path));
}

BaseMesh * MeshManager::getMesh(int id)
{
	return scene_meshes[id];
}

GuiMesh * MeshManager::getImGuiMesh()
{
	return MeshManager::imgui;
}

void MeshManager::updateImGuiBuffers(UiDrawData imgui)
{
	MeshManager::imgui->updateMeshData(imgui);
}

void MeshManager::cleanUp()
{
	for (auto mesh : scene_meshes) {
		delete mesh;
	}
	delete imgui;
}
