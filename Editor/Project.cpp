#include "Project.h"
#include "..\\VkEngine\VkEngine.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <glm/gtc/type_ptr.hpp>

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
	std::string name;
	std::string active_scene;
	std::vector<std::string> scenes;
};

Project::Project(const char* project_dir) : data(new Project::_data())
{
	json project;
	this->data->project_dir = project_dir;
	std::ifstream i((std::string(project_dir) + "proj_config.json").c_str());
	i >> project;
	this->data->name = project["name"];
	//std::cout << "Project JSON: \n" << this->data->project << std::endl;
	for (auto scene_id : project["scenes"]) {
		data->scenes.push_back(scene_id);
	}
}

void Project::load()
{
	for (const auto & entry : fs::directory_iterator(this->data->project_dir + ASSETS_DIR + MESH_DIR))
		vkengine::loadMesh(entry.path().filename().string(), entry.path().string());
	for (const auto & entry : fs::directory_iterator(this->data->project_dir + ASSETS_DIR + TEXTURE_DIR))
		vkengine::loadTexture(entry.path().filename().string(), entry.path().string());

	for (const auto & scene_id : this->data->scenes) {
		json scene;
		std::ifstream str((std::string(this->data->project_dir) + scene_id + ".json").c_str());
		str >> scene;
		vkengine::createScene(scene["id"], scene["title"]);
		vkengine::Scene3D* s = vkengine::getScene(scene["id"]);

		auto global = scene["global_light"];
		s->globalLight = {
		{global["position"][0], global["position"][1], global["position"][2], 1.0f},
		{ global["color"][0], global["color"][1], global["color"][2], 1.0f },
		{global["power"], global["power"], global["power"], global["power"]	}
		};

		for (const auto & light : scene["lights"]) {
			vkengine::PointLightInfo i = {light["name"], 
				{light["position"][0], light["position"][1], light["position"][2]},
				{light["color"][0], light["color"][1], light["color"][2]},
				light["power"] };
			s->addLight(i);
		}
		for (const auto & camera : scene["cameras"]) {
			s->addCamera(camera["name"], 
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
		for (const auto & obj : scene["objects"])
		{
			json trans = obj["transformation"];

			vkengine::ObjTransformation t = {};
			float position[] = { trans["pos"][0], trans["pos"][1], trans["pos"][2] };
			float rotation_vector[] = { trans["EulerAngles"][0], trans["EulerAngles"][1], trans["EulerAngles"][2] };
			float scale = trans["scale"];
			t.position = glm::make_vec3(position);
			t.eulerAngles = glm::make_vec3(rotation_vector);
			t.scale_factor = scale;

			vkengine::ObjectInitInfo obj_info = {};
			obj_info.name = obj["name"];
			obj_info.mesh_name = obj["mesh"];
			obj_info.texture_name = obj["texture"];
			obj_info.reflective = obj["reflective"];
			obj_info.transformation = t;
			s->addObject(obj_info);
		}
		this->data->active_scene = scene["id"];
		vkengine::loadScene(scene["id"]);
	}
}

//std::string Project::getActiveScene()
//{
//	return this->data->active_scene;
//}

void Project::save()
{
	// Main project settings
	json save;
	save["name"] = this->data->name;
	save["active-scene"] = vkengine::getActiveScene()->getId();
	save["scenes"] = vkengine::list_scenes();
	std::ofstream save_file((std::string(this->data->project_dir) + "proj_config.json").c_str());
	save_file << std::setw(4) << save << std::endl;

	// scene files
	for (auto & s : vkengine::list_scenes()) {
		auto scene = vkengine::getScene(s);
		json s_save;
		s_save["id"] = s;
		s_save["title"] = scene->name;
		s_save["def-camera"] = scene->current_camera;
		// Dumping cameras
		auto cams_ids = scene->listCameras();
		std::vector<json> cams;
		for (auto & id : cams_ids) {
			float *vertex;
			std::vector<float> v;
			auto cam = scene->getCamera(id);
			json j;
			j["name"] = cam->name;
			vertex = glm::value_ptr(cam->getViewSetup().position);
			v.assign(vertex, vertex + 3);
			j["position"] = v;
			vertex = glm::value_ptr(cam->getViewSetup().target);
			v.assign(vertex, vertex + 3);
			j["target"] = v; 
			vertex = glm::value_ptr(cam->getViewSetup().upVector);
			v.assign(vertex, vertex + 3);
			j["up-vector"] = v;
			j["aspect"] = cam->getPerspectiveSetup().aspect;
			j["fovY"] = cam->getPerspectiveSetup().fovY;
			j["near"] = cam->getPerspectiveSetup().near;
			j["far"] = cam->getPerspectiveSetup().far;
			cams.push_back(j);
		}
		s_save["cameras"] = cams;

		// Dumping lights
		{
			// Global
			float* vertex;
			std::vector<float> v;
			json g;
			g["name"] = "global";
			vertex = glm::value_ptr(scene->globalLight.position);
			v.assign(vertex, vertex + 3);
			g["position"] = v;
			vertex = glm::value_ptr(scene->globalLight.color);
			v.assign(vertex, vertex + 3);
			g["color"] = v;
			g["power"] = scene->globalLight.power.w;
			s_save["global_light"] = g;
			// Point lights
			std::vector<json> lights;
			auto lights_ids = scene->listLights();
			for (auto& id : lights_ids) 			{
				auto light = scene->getLight(id);
				json j;
				j["name"] = light->name;
				vertex = glm::value_ptr(light->getData().position);
				v.assign(vertex, vertex + 3);
				j["position"] = v;
				vertex = glm::value_ptr(light->getData().color);
				v.assign(vertex, vertex + 3);
				j["color"] = v;
				j["power"] = light->getData().power.w;
				lights.push_back(j);
			}
			s_save["lights"] = lights;
		}


		// Dumping Objects		
		auto objs_ids = scene->listObjects();
		std::vector<json> objects;
		for (auto & id : objs_ids) {
			float *vertex;
			std::vector<float> v;
			auto obj = scene->getObject(id);
			json j, trans;
			j["name"] = obj->name;
			j["mesh"] = obj->getMeshName();
			j["reflective"] = obj->reflective;
			j["texture"] = obj->getTextureName();
			// transformation info			
				vertex = glm::value_ptr(obj->getObjTransform().position);
				v.assign(vertex, vertex + 3);
				trans["pos"] = v;
				trans["scale"] = obj->getObjTransform().scale_factor;
				vertex = glm::value_ptr(obj->getObjTransform().eulerAngles);
				v.assign(vertex, vertex + 3);
				trans["EulerAngles"] = v;
				j["transformation"] = trans;
			objects.push_back(j);
		}
		s_save["objects"] = objects;

		// Write on file
		std::ofstream save_file((std::string(this->data->project_dir) + s + ".json").c_str());
		save_file << std::setw(4) << s_save << std::endl;
	}
}

Project::~Project() = default;
