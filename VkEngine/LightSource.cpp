#include "LightSource.h"


using namespace vkengine;

LightSource::LightSource(std::string id, std::string name, 
	glm::vec3 pos, glm::vec3 color, float power)
	: SceneElement(id, name)
{
	this->light_data.position = glm::vec4(pos, 1.0f);
	this->light_data.color = glm::vec4(color, 1.0f);
	this->light_data.power = glm::vec4(power);
}

LightData& LightSource::getData()
{
	return this->light_data;
}


LightSource::~LightSource()
{
}
