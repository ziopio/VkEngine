#pragma once

#include "Libraries/frustum.hpp"

namespace vkengine
{
	constexpr const float CAMERA_NORMAL_SPEED = 10.0f;
	constexpr const float CAMERA_FASTER_SPEED = 100.0f;

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
		Camera(ViewSetup view, PerspectiveSetup perspective);
		//updates camera values and returns its lookAt matrix
		glm::mat4 setCamera();
		glm::mat4 getProjection();
		bool checkFrustum(glm::vec3 pos, float radius);
		void updateAspectRatio(float width, float height);
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
		PerspectiveSetup projection;
		ViewSetup view;
		vks::Frustum frustum;
		bool foreward = false, back = false, left = false, right = false;
		bool wasd_movement_mutex = false;
		float camera_speed;
		double camera_timer;
		double debug_timer;
	};

}
