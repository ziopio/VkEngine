#include "stdafx.h"
#include "Camera.h"

using namespace glm;
using namespace vkengine;

Camera::Camera(float height, float width, ViewSetup view, PerspectiveSetup perspective)
{
	screenCenter = vec2(width/2,height/2);
	oldMousePos = screenCenter;
	camera_timer = clock();
	debug_timer = clock();
	this->view = view;
	this->projection = perspective;
	this->camera_speed = CAMERA_NORMAL_SPEED;
	this->status = CameraMode::FREE_CAM;
}

mat4 Camera::setCamera()
{
	double debug_time = (clock() - debug_timer) / (double)CLOCKS_PER_SEC;
	manageMotion((clock() - camera_timer) / (double)CLOCKS_PER_SEC);
	camera_timer = clock();

	if (oldMousePos != screenCenter)
	{ 
		//glfwSetCursorPos(window, screenCenter.x, screenCenter.y);
		oldMousePos = screenCenter;
	}
	//
	//if (debug_time > 1.0) {
	//	debug_timer = clock();
	//	printf("DEBUG_CAMERA:\n");
	//	printf("Target vector: %f,%f,%f\n",view.target.x, view.target.y, view.target.z);
	//	printf("Camera Position: %f,%f,%f\n", view.position.x, view.position.y, view.position.z);
	//	printf("UP: %f,%f,%f\n", view.upVector.x, view.upVector.y, view.upVector.z);
	//	printf("SPEED: %f\n",this->camera_speed);
	//}
	return lookAt(view.position,view.target,view.upVector);
}

mat4 Camera::getProjection()
{
	return glm::perspective(glm::radians(projection.fovY), projection.aspect, projection.near, projection.far);
}

void Camera::updateScreenCenter(float height, float width)
{
	this->projection.aspect = width / height;
	this->screenCenter = vec2(width / 2, height / 2);
}

void Camera::mouseRotation(float x, float y)
{
	if (wasd_movement_mutex) { 
		return; 
	}
	const float ROTATION_CONSTANT = 0.001f;
	oldMousePos = { x, y };

	vec2 motionVec = vec2(x - screenCenter.x,y - screenCenter.y);

	//Rotazione orizzontale
	vec3 direction = view.target - view.position;
	switch (status)
	{
	case CameraMode::UNLOCKED: // imposto una matrice di rotazione orizzontale attorno al vettore UP
		view.target = view.position + normalize(glm::mat3(glm::rotate(glm::mat4(1.0f), -motionVec.x * ROTATION_CONSTANT, view.upVector)) * direction);
		break;
	case CameraMode::FREE_CAM: 	// imposto una matrice di rotazione orizzontale attorno all'azimut
		view.target = view.position + normalize(glm::mat3(glm::rotate(glm::mat4(1.0f), -motionVec.x * ROTATION_CONSTANT, vec3(0.0f,1.0f,0.0f))) * direction);
		view.upVector = normalize(glm::mat3(glm::rotate(glm::mat4(1.0f), -motionVec.x * ROTATION_CONSTANT, vec3(0.0f, 1.0f, 0.0f))) * view.upVector); // allineo l'up vector
		break;
	}
	
	//Rotazione Verticale
	direction = view.target - view.position;
	//Attorno al Vettore: prodotto vettoriale tra upVector e direzione
	vec3 horizontalVec = glm::cross(direction, view.upVector);

	glm::mat3 vertical_ROT_MATRIX = glm::mat3(glm::rotate(glm::mat4(1.0f), -motionVec.y * ROTATION_CONSTANT, horizontalVec));
	view.target = view.position + vertical_ROT_MATRIX * direction;
	//devo applicare la rotazione verticale anche al vettore UP
	view.upVector = normalize(vertical_ROT_MATRIX * view.upVector);
}

void Camera::moveCameraForeward()
{
	foreward = true;
}

void Camera::moveCameraLeft()
{
	left = true;
}

void Camera::moveCameraRight()
{
	right = true;
}

void Camera::moveCameraBack()
{
	back = true;
}

void Camera::stopCameraForeward()
{
	foreward = false;
}

void Camera::stopCameraLeft()
{
	left = false;
}

void Camera::stopCameraRight()
{
	right = false;
}

void Camera::stopmoveCameraBack()
{
	back = false;
}

void Camera::fastSpeedCamera()
{
	this->camera_speed = CAMERA_FASTER_SPEED;
}

void Camera::normalSpeedCamera()
{
	this->camera_speed = CAMERA_NORMAL_SPEED;
}

void Camera::reset()
{
	this->view.position = vec3(0.0f,0.0f,-10.0f);
	this->view.target = vec3(0.0f, 0.0f, 0.0f);
	this->view.upVector = vec3(0.0f, 1.0f, 0.0f);
}

Camera::~Camera()
{
}

void Camera::manageMotion(double elapsed_time)
{
	wasd_movement_mutex = true;
	vec3 direction = view.target - view.position;
	if (foreward) {
		//position.x= position.x + CAMERA_SPEED * elapsed_time;
		//position.y = position.y + CAMERA_SPEED * elapsed_time;
		//position.z = position.z + CAMERA_SPEED * elapsed_time;
		
		view.position += direction * this->camera_speed * (float)elapsed_time;
		view.target += direction * this->camera_speed  * (float)elapsed_time;
	}
	if (back) {
		view.position -= direction * this->camera_speed  * (float)elapsed_time;
		view.target -= direction * this->camera_speed  * (float)elapsed_time;
		//position.x = position.x - CAMERA_SPEED * elapsed_time;
		//position.y = position.y - CAMERA_SPEED * elapsed_time;
		//position.z = position.z - CAMERA_SPEED * elapsed_time;
	}
	vec3 slide_vector = glm::cross(direction, view.upVector) * this->camera_speed * (float)elapsed_time;
	if (left) {
		view.position -= slide_vector;
		view.target -= slide_vector ;
	}
	if (right) {
		view.position += slide_vector;
		view.target += slide_vector;
	}
	wasd_movement_mutex = false;
}
