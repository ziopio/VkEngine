#include "stdafx.h"
#include "Camera.h"
#include "VkEngine.h"

using namespace glm;
using namespace vkengine;

constexpr const float NORMAL_SPEED = 10.0f;
constexpr const float FASTER_SPEED = 100.0f;
constexpr const float ROTATION_CONSTANT = 0.005f;

vkengine::Camera::Camera(std::string id, std::string name, ViewSetup view, PerspectiveSetup perspective) : 
	SceneElement( id,  name)
{
	camera_timer = clock();
	debug_timer = clock();
	this->view = view;
	this->projection = perspective;
	this->camera_speed = NORMAL_SPEED;
	this->status = CameraMode::FREE_CAM;
}

ViewSetup & Camera::getViewSetup()
{
	return this->view;
}

void vkengine::Camera::rotate_FPS_style(glm::vec2 delta)
{
	if (wasd_movement_mutex) {
		return;
	}
	

	//Rotazione orizzontale
	vec3 direction = view.target - view.position;
	switch (status)
	{
	case CameraMode::UNLOCKED: // imposto una matrice di rotazione orizzontale attorno al vettore UP
		view.target = view.position + normalize(glm::mat3(glm::rotate(glm::mat4(1.0f), -delta.x * ROTATION_CONSTANT, view.upVector)) * direction);
		break;
	case CameraMode::FREE_CAM: 	// imposto una matrice di rotazione orizzontale attorno all'azimut
		view.target = view.position + normalize(glm::mat3(glm::rotate(glm::mat4(1.0f), -delta.x * ROTATION_CONSTANT, vec3(0.0f,1.0f,0.0f))) * direction);
		view.upVector = normalize(glm::mat3(glm::rotate(glm::mat4(1.0f), -delta.x * ROTATION_CONSTANT, vec3(0.0f, 1.0f, 0.0f))) * view.upVector); // allineo l'up vector
		break;
	}

	//Rotazione Verticale
	direction = view.target - view.position;
	//Attorno al Vettore: prodotto vettoriale tra upVector e direzione
	vec3 horizontalVec = glm::cross(direction, view.upVector);

	glm::mat3 vertical_ROT_MATRIX = glm::mat3(glm::rotate(glm::mat4(1.0f), -delta.y * ROTATION_CONSTANT, horizontalVec));
	view.target = view.position + vertical_ROT_MATRIX * direction;
	//devo applicare la rotazione verticale anche al vettore UP
	view.upVector = normalize(vertical_ROT_MATRIX * view.upVector);
	
}

mat4 Camera::setCamera()
{
	this->manageMotion(vkengine::unified_delta_time);
	glm::mat4 V = lookAt(view.position,view.target,view.upVector);
	this->frustum.update(this->getProjection() * V );
	return V;
}

mat4 Camera::getProjection()
{
	return glm::perspective(glm::radians(projection.fovY), projection.aspect, projection.near, projection.far);
}

bool vkengine::Camera::checkFrustum(glm::vec3 pos, float radius)
{
	return this->frustum.checkSphere(pos, radius);
}

void Camera::updateAspectRatio(float width, float height)
{
	this->projection.aspect = width / height;
	//this->screenCenter = vec2(width / 2, height / 2);
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

void Camera::stopCameraBack()
{
	back = false;
}

void Camera::fastSpeedCamera()
{
	this->camera_speed = FASTER_SPEED;
}

void Camera::normalSpeedCamera()
{
	this->camera_speed = NORMAL_SPEED;
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
