#pragma once
#include "Camera.h"

class SurfaceOwner;

class Direction
{
public:
	static void initialize();
	static Camera* getCurrentCamera();
	static void addCamera(glm::vec3 position, glm::vec3 target, glm::vec3 upVector);
	// Increments the camera_index
	static void cycleCameras();
	static int countCameras();
	static void updateCamerasScreenSize(int width, int height);
	static void cleanUp();
private:
	static int w_height, w_width;
	static std::vector<Camera*> cameras;
	static int camera_index;
	static bool ready;
};

