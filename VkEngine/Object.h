#pragma once

namespace vkengine
{
	enum MaterialType {
		SAMPLE,
		PHONG,
		UI,
		LAST // DUMMY TOKEN leave at END!
	};

	typedef struct {
		float position[3];
		/* Axis of the rotation	*/
		float rotation_vector[3];
		float angularSpeed;
		float scale_vector[3];
		float scale_factor;
	} ObjTransformation;

	typedef struct {
		std::string mesh_id;
		int material_id;
		std::string texture_id;
		ObjTransformation transformation;
	} ObjectInitInfo;

	class Object
	{
	public:
		Object(std::string mesh_id, MaterialType material, std::string texture_id, ObjTransformation transform);
		glm::mat4 getMatrix();
		float getScale();
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
}



