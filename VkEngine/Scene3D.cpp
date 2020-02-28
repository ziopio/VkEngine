#include "Scene3D.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>

using namespace glm;
using namespace vkengine;

Scene3D::Scene3D(std::string id)
{
	this->id = id;
}

void Scene3D::addCamera(std::string id, ViewSetup view, PerspectiveSetup perspective)
{
	this->cameras.insert( { id, Camera(view, perspective) } );
	this->current_camera = id;
}

Camera* vkengine::Scene3D::getCurrentCamera()
{
	return &cameras.at(current_camera);
}

void Scene3D::addObject(vkengine::ObjectInitInfo obj_info)
{
	objects.insert({ obj_info.id,
		Object3D(obj_info.mesh_id, obj_info.material_id, obj_info.texture_id, obj_info.transformation) } );
}

Object3D* Scene3D::getObject(std::string id)
{
	return &objects.at(id);
}

std::vector<std::string> vkengine::Scene3D::listObjects()
{
	std::vector<std::string> keys;
	for (auto entry : objects) {
		keys.push_back(entry.first);
	}
	return keys;
}

void Scene3D::removeObject(std::string id)
{
	objects.erase(id);
}

void Scene3D::addLight(vkengine::PointLightInfo info)
{
	lights.insert({ info.id, LightSource(glm::make_vec3(info.position), glm::make_vec3(info.color), info.power) } );
}

LightSource* Scene3D::getLight(std::string id)
{
	return &lights.at(id);
}

std::vector<std::string> vkengine::Scene3D::listLights()
{
	std::vector<std::string> keys;
	for (auto entry : lights) {
		keys.push_back(entry.first);
	}
	return keys;
}

void Scene3D::removeLight(std::string id)
{
	lights.erase(id);
}

Scene3D::~Scene3D() = default;
