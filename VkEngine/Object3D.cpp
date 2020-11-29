#include "Object3D.h"
#include "VkEngine.h"
#include "commons.h"

using namespace glm; 
using namespace vkengine;

Object3D::Object3D(unsigned id, std::string name, std::string mesh_id,
	std::string texture, ObjTransformation transform)
	: SceneElement(id, name)
{
	this->mesh_name = mesh_id;
	this->texture_name = texture;
	this->transform = transform;
}

glm::mat4 Object3D::getMatrix()
{
	return glm::translate(glm::mat4(1), transform.position) * 
		glm::toMat4(glm::quat(transform.eulerAngles * glm::pi<float>() / 180.f)) * 
		glm::scale(glm::mat4(1.f),glm::vec3(transform.scale_factor));
}

ObjTransformation & vkengine::Object3D::getObjTransform()
{
	return this->transform;
}

float vkengine::Object3D::getBoundingRadius()
{ // big brain code...
	return this->transform.scale_factor;
}

std::string Object3D::getMeshName()
{
	return mesh_name;
}

std::string Object3D::getTextureName()
{
	return texture_name;
}

Object3D::~Object3D()
{
}
