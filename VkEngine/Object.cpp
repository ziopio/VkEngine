#include "stdafx.h"
#include "Object.h"
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Object::Object(int mesh_id, MaterialType material, int texture_id, ObjTransformation transform)
{
	this->mesh_id = mesh_id;
	this->texture_id = texture_id;
	this->material = material;
	this->transform = transform;
	glm::mat4 traslation = glm::translate(glm::mat4(), glm::make_vec3(transform.position));
	glm::mat4 scale = glm::scale(glm::mat4(), glm::make_vec3(transform.scale_vector) * transform.scale_factor);
	this->ObjMatrix = traslation * scale;
}

glm::mat4 Object::getMatrix()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	this->ObjMatrix =  glm::rotate(this->ObjMatrix, transform.angularSpeed, glm::make_vec3(transform.rotation_vector));
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

int Object::getMeshId()
{
	return mesh_id;
}

MaterialType Object::getMatType()
{
	return this->material;
}

int Object::getTextureId()
{
	return mesh_id;
}

Object::~Object()
{
}
