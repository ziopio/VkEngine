#include "commons.h"
#include "MeshManager.h"
#include "VkEngine.h"
#include "SwapChain.h"

using namespace vkengine;

std::unordered_map<std::string, Mesh3D*> MeshManager::scene_meshes;
std::vector<GuiMesh*> MeshManager::per_frame_imguis;

void MeshManager::init()
{
	MeshManager::per_frame_imguis.resize(SwapChainMng::get()->getImageCount());
	for (int i = 0; i < SwapChainMng::get()->getImageCount(); i++) {
		MeshManager::per_frame_imguis[i] = new GuiMesh();
	}
}

void MeshManager::addMesh(std::string id, std::string mesh_path)
{
	scene_meshes[id] = new Mesh3D(mesh_path);
}

Mesh3D * MeshManager::getMesh(std::string id)
{
	return scene_meshes[id];
}

std::vector<std::string> MeshManager::listLoadedMeshes()
{
	std::vector<std::string> mesh_ids;
	for (auto id : MeshManager::scene_meshes) {
		mesh_ids.push_back(id.first);
	}
	return mesh_ids;
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
