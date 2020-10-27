#include "Scene3D.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>

using namespace glm;
using namespace vkengine;

constexpr const unsigned INITIAL_CAPACITY = 32;

unsigned getNewUniversalID() {
	static unsigned next_id = 0;
	return next_id++;
}

Scene3D::Scene3D(std::string id, std::string name)
{
	this->object_capacity = INITIAL_CAPACITY;
	this->id = id;
	this->name = name;
}

void Scene3D::addCamera(std::string name, ViewSetup view, PerspectiveSetup perspective)
{
	unsigned id = getNewUniversalID();
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
	unsigned id = getNewUniversalID();
	objects.insert({ id,
		Object3D(id, obj_info.name, obj_info.mesh_name, obj_info.texture_name, obj_info.transformation) } );
	if (objects.size() > object_capacity) object_capacity *= 2;
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
	if (objects.size() < object_capacity/2) object_capacity /= 2;
}

void Scene3D::addLight(vkengine::PointLightInfo info)
{
	unsigned id = getNewUniversalID();
	point_lights.insert({ id, PointLight(id, info.name, glm::make_vec3(info.position), glm::make_vec3(info.color), info.power) } );
}

PointLight* Scene3D::getLight(unsigned id)
{
	return &point_lights.at(id);
}

std::vector<unsigned> vkengine::Scene3D::listLights()
{
	std::vector<unsigned> keys;
	for (auto entry : point_lights) {
		keys.push_back(entry.first);
	}
	return keys;
}

void Scene3D::removeLight(unsigned id)
{
	point_lights.erase(id);
}

std::vector<SceneElement*> vkengine::Scene3D::getAllElements()
{
	std::vector<SceneElement*> vec;
	for (auto entry : objects) {
		vec.push_back(&objects.at(entry.first));
	}
	for (auto entry : point_lights) {
		vec.push_back(&point_lights.at(entry.first));
	}
	for (auto entry : cameras) {
		vec.push_back(&cameras.at(entry.first));
	}
	return vec;
}

Scene3D::~Scene3D() = default;
