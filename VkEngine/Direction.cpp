#include "stdafx.h"
#include "Direction.h"

using namespace glm;

int  Direction::w_height, Direction::w_width;
std::vector<Camera*> Direction::cameras;
int Direction::camera_index;
bool Direction::ready;


void Direction::initialize()
{
	Direction::ready = true;
	Direction::addCamera(glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 10.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
}

Camera* Direction::getCurrentCamera()
{
	if(!ready || Direction::cameras.size() <= 0){	throw std::runtime_error("Camera Direction not ready!");}
	return Direction::cameras[camera_index];
}

void Direction::addCamera(vec3 position, vec3 target, vec3 upVector)
{
	PerspectiveSetup proj = {};
	proj.fovY = 45.0f;
	proj.aspect = (float) w_width / w_height;
	proj.near = 1.0f;
	proj.far = 10000.0f;
	Direction::cameras.push_back(new Camera(w_height, w_width, ViewSetup{ position, target, upVector }, proj));
}

void Direction::cycleCameras()
{
	camera_index++;
	if (camera_index >= cameras.size()) {
		camera_index = 0;
	}
}

int Direction::countCameras()
{
	return Direction::cameras.size();
}

void Direction::updateCamerasScreenSize(int width, int height)
{
	Direction::w_width = width;
	Direction::w_height = height;
	for (auto cam : cameras) {
		cam->updateScreenCenter(w_height, w_width);
	}
}

void Direction::cleanUp()
{
	for (auto cam : cameras) {
		delete cam;
	}
}
