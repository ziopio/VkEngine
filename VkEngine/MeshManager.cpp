#include "commons.h"
#include "MeshManager.h"
#include "VkEngine.h"
#include "SwapChain.h"

using namespace vkengine;

unsigned MeshManager::mesh_capacity = SUPPORTED_MESH_COUNT;
std::unordered_map<std::string, unsigned> MeshManager::mesh_ids;
std::vector<Mesh3D*> MeshManager::mesh_library;
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
	mesh_library.push_back(new Mesh3D(mesh_path));
	mesh_ids[id] = mesh_library.size() - 1;
}

Mesh3D * MeshManager::getMesh(unsigned id)
{
	return mesh_library[id];
}

Mesh3D * MeshManager::getMesh(std::string string_id)
{
	return mesh_library[mesh_ids[string_id]];
}

unsigned MeshManager::getMeshID(std::string string_id)
{
	return mesh_ids[string_id];
}

std::vector<Mesh3D*> MeshManager::getMeshLibrary()
{
	return std::vector<Mesh3D*>(mesh_library);
}

std::vector<std::string> MeshManager::listLoadedMeshes()
{
	std::vector<std::string> mesh_ids;
	for (auto id : MeshManager::mesh_ids) {
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
	mesh_ids.clear();
	for (auto mesh : mesh_library) {
		delete mesh;
	}
	for (auto imgui : per_frame_imguis) {
		delete imgui;
	}
}
