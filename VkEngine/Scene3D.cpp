#include "Scene3D.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>

using namespace glm;
using namespace vkengine;

Scene3D::Scene3D(std::string id, std::string name)
{
	this->id = id;
	this->name = name;
}

void Scene3D:: addCamera(unsigned id, std::string name, ViewSetup view, PerspectiveSetup perspective)
{
	this->cameras.insert( { id, Camera( id, name,view, perspective) } );
	this->current_camera = id;
}

Camera* vkengine::Scene3D::getCamera(unsigned id)
{
	return &cameras.at(id);
}

std::vector<unsigned> vkengine::Scene3D::listCameras()
{
	std::vector<unsigned> keys;
	for (auto entry : cameras) {
		keys.push_back(entry.first);
	}
	return keys;
}

void Scene3D::addObject(vkengine::ObjectInitInfo obj_info)
{
	objects.insert({ obj_info.id,
		Object3D(obj_info.id, obj_info.name, obj_info.mesh_id, obj_info.material, obj_info.transformation) } );
}

Object3D* Scene3D::getObject(unsigned id)
{
	return &objects.at(id);
}

std::vector<unsigned> vkengine::Scene3D::listObjects()
{
	std::vector<unsigned> keys;
	for (auto entry : objects) {
		keys.push_back(entry.first);
	}
	return keys;
}

void Scene3D::removeObject(unsigned id)
{
	objects.erase(id);
}

void Scene3D::addLight(vkengine::PointLightInfo info)
{
	lights.insert({ info.id, LightSource(info.id, info.name, glm::make_vec3(info.position), glm::make_vec3(info.color), info.power) } );
}

LightSource* Scene3D::getLight(unsigned id)
{
	return &lights.at(id);
}

std::vector<unsigned> vkengine::Scene3D::listLights()
{
	std::vector<unsigned> keys;
	for (auto entry : lights) {
		keys.push_back(entry.first);
	}
	return keys;
}

void Scene3D::removeLight(unsigned id)
{
	lights.erase(id);
}

std::vector<SceneElement*> vkengine::Scene3D::getAllElements()
{
	std::vector<SceneElement*> vec;
	for (auto entry : objects) {
		vec.push_back(&objects.at(entry.first));
	}
	for (auto entry : lights) {
		vec.push_back(&lights.at(entry.first));
	}
	for (auto entry : cameras) {
		vec.push_back(&cameras.at(entry.first));
	}
	return vec;
}

Scene3D::~Scene3D() = default;
