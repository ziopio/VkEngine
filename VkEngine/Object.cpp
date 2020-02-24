#include "stdafx.h"
#include "Object.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Object::Object(std::string mesh_id, MaterialType material, std::string texture_id, ObjTransformation transform)
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

glm::mat4 Object::getMatrix()
{
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	startTime = currentTime;
	this->ObjMatrix = glm::rotate(ObjMatrix, time * glm::radians(transform.angularSpeed), glm::make_vec3(transform.rotation_vector));
	return this->ObjMatrix;
}

ObjTransformation Object::getInfo()
{
	return this->transform;
}

glm::vec3 Object::getPos()
{
	return this->position;
}

std::string Object::getMeshId()
{
	return mesh_id;
}

MaterialType Object::getMatType()
{
	return this->material;
}

std::string Object::getTextureId()
{
	return texture_id;
}

Object::~Object()
{
}
