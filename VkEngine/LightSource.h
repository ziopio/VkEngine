#pragma once
#include "SceneElement.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <string>

namespace vkengine 
{

	typedef struct {
		std::string name;
		float position[3];
		float color[3];
		float power;
	} PointLightInfo;


	struct LightData {
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 power;
	};

	class PointLight : public SceneElement
	{
	public:
		PointLight(unsigned id, std::string name,
			glm::vec3 pos, glm::vec3 color, float power);
		LightData& getData();
		~PointLight();
	private:
		LightData light_data;
	};
}


