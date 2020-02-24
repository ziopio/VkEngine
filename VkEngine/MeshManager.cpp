#include "stdafx.h"
#include "MeshManager.h"
#include "VkEngine.h"

using namespace vkengine;

std::unordered_map<std::string, Mesh*> MeshManager::scene_meshes;
std::vector<GuiMesh*> MeshManager::per_frame_imguis;

void MeshManager::init(unsigned swapchain_image_count)
{
	MeshManager::per_frame_imguis.resize(swapchain_image_count);
	for (int i = 0; i < swapchain_image_count; i++) {
		MeshManager::per_frame_imguis[i] = new GuiMesh();
	}
}

void MeshManager::addMesh(std::string id, std::string mesh_path)
{
	scene_meshes[id] = new Mesh(mesh_path);
}

BaseMesh * MeshManager::getMesh(std::string id)
{
	return scene_meshes[id];
}

GuiMesh * MeshManager::getImGuiMesh(unsigned imageIndex)
{
	return MeshManager::per_frame_imguis[imageIndex];
}

void MeshManager::updateImGuiBuffers(UiDrawData imgui, unsigned imageIndex)
{
	MeshManager::per_frame_imguis[imageIndex]->updateMeshData(imgui);
}

void MeshManager::cleanUp()
{
	for (auto mesh : scene_meshes) {
		delete mesh.second;
	}
	for (auto imgui : per_frame_imguis) {
		delete imgui;
	}
}
