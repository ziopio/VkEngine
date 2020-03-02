#include "stdafx.h"
#include "Object3D.h"

using namespace glm; 
using namespace vkengine;

Object3D::Object3D(std::string id, std::string name, std::string mesh_id, 
	MaterialType material, std::string texture_id, ObjTransformation transform)
	: SceneElement(id, name)
{
	this->mesh_id = mesh_id;
	this->texture_id = texture_id;
	this->material = material;
	this->transform = transform;
	this->position = glm::make_vec3(transform.position);
	glm::mat4 traslation = glm::translate(glm::mat4(1.f), this->position);
	glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::make_vec3(transform.scale_vector) * transform.scale_factor);
	this->ObjMatrix = traslation * scale;
}

glm::mat4 Object3D::getMatrix()
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	startTime = currentTime;
	this->ObjMatrix = glm::rotate(ObjMatrix, time * glm::radians(transform.angularSpeed), glm::make_vec3(transform.rotation_vector));
	return this->ObjMatrix;
}

float Object3D::getScale()
{
	return this->transform.scale_factor;
}

glm::vec3 Object3D::getPos()
{
	return this->position;
}

std::string Object3D::getMeshId()
{
	return mesh_id;
}

MaterialType Object3D::getMatType()
{
	return this->material;
}

std::string Object3D::getTextureId()
{
	return texture_id;
}

Object3D::~Object3D()
{
}
