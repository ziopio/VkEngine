#include <stdio.h>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include "VkEngine.h"
#include "Object.h"
#include "LightSource.h"

#define NDEBUG

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
static const std::string SKYDOME_PATH = "Models/dome.obj";
static const std::string SKYDOME_TEXTURE = "Textures/space.jpg";

static const std::string SU33_PATH = "Models/Su-33.obj";
static const std::string SU33_TEXTURE = "Textures/Su-33.jpg";

static const std::string PUZZLE_PATH = "Models/puzzle.obj";
static const std::string PUZZLE_SMOOTH_PATH = "Models/puzzle_smooth.obj";
static const std::string PUZZLE_TEXT = "Textures/gold_low.jpg";

static const std::string CONE_PATH = "Models/cone.obj";
static const std::string CHALET_PATH = "Models/chalet.obj";
static const std::string CUBE_PATH = "Models/cube.obj";
static const std::string TEXTURE_PATH = "Textures/chalet.jpg";

static VkEngine* engine;
static std::vector<Object*> objects;
static std::vector<LightSource*> lights;

void loadStuff() {


	engine->loadMesh(SKYDOME_PATH);
	engine->loadTexture(SKYDOME_TEXTURE);
	engine->loadMesh(PUZZLE_SMOOTH_PATH);
	engine->loadTexture(PUZZLE_TEXT);

	Transformation transform = {};
	transform.position = glm::vec3(0);
	transform.rotation_vector = glm::vec3(0,1,0);
	transform.angularSpeed = 0.0f;
	transform.scale_vector = glm::vec3(1.0f);
	transform.scale_factor = 100.0f;
	Object* obj = new Object(0,MaterialType::SAMPLE,0,transform);
	objects.push_back(obj);

	int distanceMultiplier = 15;
	int cubicExpansion = 10;
	float rx, ry, rz;
	for (float x = -cubicExpansion; x < cubicExpansion; x++)
	{
		for (float y = -cubicExpansion; y < cubicExpansion; y++)
		{
			for (float z = -cubicExpansion; z < cubicExpansion; z++)
			{
				rx = rand() % 10 * ((rand() % 2) ? -1 : 1);
				ry = rand() % 10 * ((rand() % 2) ? -1 : 1);
				rz = rand() % 10 * ((rand() % 2) ? -1 : 1);
				transform.position = glm::vec3(x*distanceMultiplier, y*distanceMultiplier, z*distanceMultiplier);
				transform.rotation_vector = glm::vec3(rx, ry, rz);
				transform.angularSpeed = 30.0f;
				transform.scale_vector = glm::vec3(1.0f);
				transform.scale_factor = 1.0f;
				Object* obj = new Object(1, MaterialType::PHONG, 1, transform);
				objects.push_back(obj);
			}
		}
	}
	// 9 lights
	lights.push_back(new LightSource(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(100.0f, 100.0f, 100.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(100.0f, 100.0f, -100.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(100.0f, -100.0f, 100.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(100.0f, -100.0f, -100.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(-100.0f, 100.0f, 100.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(-100.0f, 100.0f, -100.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(-100.0f, -100.0f, 100.0f), glm::vec3(1, 1, 1), 10.f));
	lights.push_back(new LightSource(glm::vec3(-100.0f, -100.0f, -100.0f), glm::vec3(1, 1, 1), 10.f));
	engine->setObjects(objects);
	engine->setLights(lights);
}

int main() {
	HWND consoleWindow = GetConsoleWindow();
	SetWindowPos(consoleWindow, 0, 0, 530, 0, 0, SWP_NOSIZE | SWP_NOZORDER);   
	engine = new VkEngine();
	if (enableValidationLayers) {
		engine->validation = true;
	}
	try {
		engine->initialize();
		
		loadStuff();

		engine->renderFrame();

		engine->cleanUp();

		delete engine;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	while (getchar() != 10);
	return EXIT_SUCCESS;
}

