#pragma once
#include "SceneElement.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <string>

namespace vkengine
{

	typedef struct {
		glm::vec3 position;
		/* Axis of the rotation	*/
		glm::vec3 rotation_vector;
		float angle;
		float scale_factor;
	} ObjTransformation;

	typedef struct {
		std::string name;
		std::string mesh_name;
		std::string texture_name;
		bool reflective;
		ObjTransformation transformation;
	} ObjectInitInfo;

	class Object3D : public SceneElement
	{
	public:
		Object3D(unsigned id, std::string name, std::string mesh_id, std::string texture_id, ObjTransformation transform);
		glm::mat4 getMatrix();
		ObjTransformation & getObjTransform();
		// this is dumb and fake
		float getBoundingRadius();
		std::string getMeshName();
		std::string getTextureName();
		~Object3D();
		bool visible = true;
		bool reflective = false;
	private:
		glm::mat4 rotMatrix;
		std::string mesh_name;
		std::string texture_name;
		ObjTransformation transform;
	};
}



