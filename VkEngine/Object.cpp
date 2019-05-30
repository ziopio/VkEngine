#include "stdafx.h"
#include "Object.h"

using namespace glm;

Object::Object(int mesh_id, MaterialType material, int texture_id, Transformation transform)
{
	this->mesh_id = mesh_id;
	this->texture_id = texture_id;
	this->material = material;
	this->transform = transform;
}

glm::mat4 Object::getMatrix()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	return  glm::translate(glm::mat4(1.0f), transform.position) * rotate(mat4(1.0f), glm::radians( time * transform.angularSpeed),this->transform.rotation_vector) * scale(mat4(1.0f),transform.scale_vector* transform.scale_factor);
}

Transformation Object::getInfo()
{
	return this->transform;
}

glm::vec3 Object::getPos()
{
	return this->transform.position;
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
