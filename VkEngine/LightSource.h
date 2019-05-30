#pragma once
#include "MaterialManager.h"

class LightSource
{
public:
	LightSource(glm::vec3 pos, glm::vec3 color, float power);
	Light getData();
	~LightSource();
private:
	Light light_data;
};

