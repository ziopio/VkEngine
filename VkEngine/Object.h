#pragma once
#include "Material.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
using namespace vkengine;


class Object
{
public:
	Object(int mesh_id, MaterialType material, int texture_id, ObjTransformation transform);
	glm::mat4 getMatrix();
	ObjTransformation getInfo();
	glm::vec3 getPos();
	int getMeshId();
	MaterialType getMatType();
	int getTextureId();
	~Object();
	bool visible = true;
private:
	glm::mat4 ObjMatrix;
	glm::vec3 position;
	int mesh_id;
	int texture_id;
	MaterialType material;
	ObjTransformation transform;
};

