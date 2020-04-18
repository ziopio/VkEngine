#pragma once
#include "SceneElement.h"
#include "Material.h"
#include <string>

namespace vkengine
{

	typedef struct {
		glm::vec3 position;
		/* Axis of the rotation	*/
		glm::vec3 rotation_vector;
		float angle;
		glm::vec3 scale_vector;
	} ObjTransformation;

	typedef struct {
		std::string name;
		std::string id;
		std::string mesh_id;
		Material material;
		ObjTransformation transformation;
	} ObjectInitInfo;

	class Object3D : public SceneElement
	{
	public:
		Object3D(std::string id, std::string name, std::string mesh_id, Material material_id, ObjTransformation transform);
		glm::mat4 getMatrix();
		ObjTransformation & getObjTransform();
		// this is dumb and fake
		float getBoundingRadius();
		std::string getMeshId();
		Material& getMaterial();
		std::string getTextureId();
		~Object3D();
		bool visible = true;
	private:
		glm::mat4 rotMatrix;
		std::string mesh_id;
		Material material;
		ObjTransformation transform;
	};
}



