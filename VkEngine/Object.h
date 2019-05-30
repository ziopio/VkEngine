#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"

struct Transformation {
	glm::vec3 position;
	/* Axis of the rotation	*/
	glm::vec3 rotation_vector;
	float angularSpeed;
	glm::vec3 scale_vector;
	float scale_factor;
};

class Object
{
public:
	Object(int mesh_id, MaterialType material, int texture_id, Transformation transform);
	glm::mat4 getMatrix();
	Transformation getInfo();
	glm::vec3 getPos();
	int getMeshId();
	MaterialType getMatType();
	int getTextureId();
	~Object();

	bool visible = true;
private:
	Transformation transform;
	int mesh_id;
	int texture_id;
	MaterialType material;
};

