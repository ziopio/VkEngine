#pragma once
#include "Material.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
using namespace vkengine;


class Object
{
public:
	Object(std::string mesh_id, MaterialType material, std::string texture_id, ObjTransformation transform);
	glm::mat4 getMatrix();
	ObjTransformation getInfo();
	glm::vec3 getPos();
	std::string getMeshId();
	MaterialType getMatType();
	std::string getTextureId();
	~Object();
	bool visible = true;
private:
	glm::mat4 ObjMatrix;
	glm::vec3 position;
	std::string mesh_id;
	std::string texture_id;
	MaterialType material;
	ObjTransformation transform;
};

