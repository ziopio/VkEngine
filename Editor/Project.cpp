#include "Project.h"
#include "..\\VkEngine\VkEngine.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>

// nlohmann/json.hpp
#include "json.hpp"
// for convenience
using json = nlohmann::json;
namespace fs = std::filesystem;

constexpr const char* ASSETS_DIR = "/Assets"; 
constexpr const char* MESH_DIR = "/Meshes"; 
constexpr const char* TEXTURE_DIR = "/Textures";

//pimpl idiom
struct Project::_data {
	std::string project_dir;
	json project;
	std::vector<json> scenes;
};

Project::Project(const char* project_dir) : data(new Project::_data())
{
	this->data->project_dir = project_dir;
	std::ifstream i((std::string(project_dir) + "proj_config.json").c_str());
	i >> this->data->project;
	std::cout << "Project JSON: \n" << this->data->project << std::endl;
	for (auto scene : this->data->project["scenes"]) {
		std::ifstream s((std::string(project_dir) + scene.get<std::string>() + ".json" ).c_str());
		s >> scene;
		data->scenes.push_back(scene);
	}
}

void Project::load()
{
	for (const auto entry : fs::directory_iterator(this->data->project_dir + ASSETS_DIR + MESH_DIR))
		vkengine::loadMesh(entry.path().filename().string(), entry.path().string());
	for (const auto & entry : fs::directory_iterator(this->data->project_dir + ASSETS_DIR + TEXTURE_DIR))
		vkengine::loadTexture(entry.path().filename().string(), entry.path().string());

	// Fix demo light
	vkengine::PointLightInfo l = {
		3,3,3,
		1,1,1,
		2
	};
	vkengine::addLight(l);
	
	// Just first scene is supported for now
	for ( const auto obj : data->scenes[0]["objects"] )
	{
		float position[] = { 0,0,0 };
		float rotation_vector[] = { -1,1,-1 };
		float scale_vector[] = { 1,1,1 };
		vkengine::ObjTransformation  t = {};
		t.angularSpeed = 30.0f; // just for testing
		std::copy(std::begin(position), std::end(position), std::begin(t.position));
		std::copy(std::begin(rotation_vector), std::end(rotation_vector), std::begin(t.rotation_vector));
		t.scale_factor = 1.;
		std::copy(std::begin(scale_vector), std::end(scale_vector), std::begin(t.scale_vector));

		vkengine::ObjectInitInfo obj_info = {};
		obj_info.mesh_id = obj["mesh"];
		obj_info.texture_id = obj["texture"];
		obj_info.material_id = vkengine::MaterialType::PHONG;
		obj_info.transformation = t;
		vkengine::addObject(obj_info);
	}
}

void Project::save()
{
}

Project::~Project()
{
	delete data;
}
