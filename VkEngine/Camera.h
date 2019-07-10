#pragma once

#define CAMERA_NORMAL_SPEED 10.0f 
#define CAMERA_FASTER_SPEED 100.0f

enum CameraMode {
	UNLOCKED,
	FREE_CAM,
	BOUND
};

struct ViewSetup {
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 upVector;
};

struct PerspectiveSetup {
	float fovY, aspect, near, far;
};

class Camera
{
public:
	Camera( float height, float width, ViewSetup view, PerspectiveSetup perspective);
	//updates camera values and returns its lookAt matrix
	glm::mat4 setCamera();
	glm::mat4 getProjection();
	void updateScreenCenter(float height, float width);
	void mouseRotation(float x, float y);
	void moveCameraForeward();
	void moveCameraLeft();
	void moveCameraRight();
	void moveCameraBack();
	void stopCameraForeward();
	void stopCameraLeft();
	void stopCameraRight();
	void stopmoveCameraBack();
	void fastSpeedCamera();
	void normalSpeedCamera();
	void reset();
	~Camera();
private:
	void manageMotion(double elapsed_milliseconds);
private:
	CameraMode status = CameraMode::FREE_CAM;
	glm::vec2 oldMousePos;
	glm::vec2 screenCenter;
	PerspectiveSetup projection;
	ViewSetup view;
	bool foreward = false, back = false, left = false, right = false;
	bool wasd_movement_mutex = false;
	float camera_speed;
	double camera_timer;
	double debug_timer;
};
