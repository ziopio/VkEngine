#pragma once
#include "SceneElement.h"
#include <string>

namespace vkengine
{
	enum MaterialType {
		SAMPLE,
		PHONG,
		UI,
		LAST // DUMMY TOKEN leave at END!
	};

	typedef struct {
		glm::vec3 position;
		/* Axis of the rotation	*/
		glm::vec3 rotation_vector;
		float angularSpeed;
		glm::vec3 scale_vector;
	} ObjTransformation;

	typedef struct {
		std::string name;
		std::string id;
		std::string mesh_id;
		MaterialType material_id;
		std::string texture_id;
		ObjTransformation transformation;
	} ObjectInitInfo;

	class Object3D : public SceneElement
	{
	public:
		Object3D(std::string id, std::string name, std::string mesh_id, MaterialType material, std::string texture_id, ObjTransformation transform);
		glm::mat4 getMatrix();
		ObjTransformation & getObjTransform();
		// this is dumb and fake
		float getBoundingRadius();
		std::string getMeshId();
		MaterialType getMatType();
		std::string getTextureId();
		~Object3D();
		bool visible = true;
	private:
		glm::mat4 rotMatrix;
		std::string mesh_id;
		std::string texture_id;
		MaterialType material;
		ObjTransformation transform;
	};
}



