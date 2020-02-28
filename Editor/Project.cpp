#include "Project.h"
#include "..\\VkEngine\VkEngine.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

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
	std::string active_scene;
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

	for (const auto scene : this->data->scenes) {
		vkengine::createScene(scene["id"]);
		vkengine::Scene3D& s = vkengine::getScene(scene["id"]);

		for (const auto light : scene["lights"]) {
			vkengine::PointLightInfo i = { light["name"], 
				{light["position"][0], light["position"][1], light["position"][2]},
				{light["color"][0], light["color"][1], light["color"][2]},
				light["power"] };
			s.addLight(i);
		}
		for (const auto camera : scene["cameras"]) {
			s.addCamera(camera["name"], 
				{
				  glm::vec3(camera["position"][0], camera["position"][1], camera["position"][2]),
				  glm::vec3(camera["target"][0], camera["target"][1], camera["target"][2]),
				  glm::vec3(camera["up-vector"][0], camera["up-vector"][1], camera["up-vector"][2])
				},
				{ camera["fovY"],
				  16.f/9.f, // fixed value for now..
				  camera["near"],
				  camera["far"]});
		}
		for (const auto obj : scene["objects"])
		{
			json trans = obj["transformation"];
			float position[] = { trans["pos"][0], trans["pos"][1], trans["pos"][2] };
			float rotation_vector[] = { trans["rot-axis"][0], trans["rot-axis"][1], trans["rot-axis"][2] };
			float scale_vector[] = { trans["scale"][0], trans["scale"][1], trans["scale"][2] };
			vkengine::ObjTransformation  t = {};
			t.angularSpeed = trans["rotation"]; // just for testing
			std::copy(std::begin(position), std::end(position), std::begin(t.position));
			std::copy(std::begin(rotation_vector), std::end(rotation_vector), std::begin(t.rotation_vector));
			t.scale_factor = 1.;
			std::copy(std::begin(scale_vector), std::end(scale_vector), std::begin(t.scale_vector));

			vkengine::ObjectInitInfo obj_info = {};
			obj_info.id = obj["name"];
			obj_info.mesh_id = obj["mesh"];
			obj_info.texture_id = obj["texture"];
			obj_info.material_id = vkengine::MaterialType::PHONG;
			obj_info.transformation = t;
			s.addObject(obj_info);
		}
		this->data->active_scene = scene["id"];
		vkengine::loadScene(scene["id"]);
	}
}

std::string Project::getActiveScene()
{
	return this->data->active_scene;
}

void Project::save()
{
}

Project::~Project() = default;
