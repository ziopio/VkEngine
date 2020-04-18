#include "stdafx.h"
#include "Object3D.h"
#include "VkEngine.h"

using namespace glm; 
using namespace vkengine;

Object3D::Object3D(std::string id, std::string name, std::string mesh_id, 
	Material material, ObjTransformation transform)
	: SceneElement(id, name)
{
	this->mesh_id = mesh_id;
	this->material = material;
	this->transform = transform;
	this->rotMatrix = glm::mat4(1.f);
}

glm::mat4 Object3D::getMatrix()
{
	//static auto startTime = std::chrono::high_resolution_clock::now();
	//auto currentTime = std::chrono::high_resolution_clock::now();
	//float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	//startTime = currentTime;
	this->rotMatrix = glm::rotate(rotMatrix, (float)vkengine::unified_delta_time * glm::radians(transform.angle), transform.rotation_vector);
	return glm::translate(glm::mat4(1), transform.position) * this->rotMatrix * glm::scale(glm::mat4(1.f), transform.scale_vector);
}

ObjTransformation & vkengine::Object3D::getObjTransform()
{
	return this->transform;
}

float vkengine::Object3D::getBoundingRadius()
{ // big brain code...
	return (this->transform.scale_vector.x +
		this->transform.scale_vector.y +
	this->transform.scale_vector.z) / 3.0f;
}

std::string Object3D::getMeshId()
{
	return mesh_id;
}

Material& Object3D::getMaterial()
{
	return this->material;
}

std::string Object3D::getTextureId()
{
	return material.texture_id;
}

Object3D::~Object3D()
{
}
