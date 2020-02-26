#pragma once

namespace vkengine 
{

	typedef struct {
		float position[3];
		float color[3];
		float power;
	} PointLightInfo;


	struct LightData {
		glm::vec4 position;
		glm::vec4 color;
		glm::vec4 power;
	};

	class LightSource
	{
	public:
		LightSource(glm::vec3 pos, glm::vec3 color, float power);
		LightData getData();
		~LightSource();
	private:
		LightData light_data;
	};
}


