#pragma once
#include "SceneElement.h"
#include "Libraries/frustum.hpp"

namespace vkengine
{



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
		float fovY;
		float aspect;
		float near;
		float far;
	};

	class Camera : public SceneElement
	{
	public:
		Camera(unsigned id, std::string name, ViewSetup view, PerspectiveSetup perspective);
		//updates camera values and returns its lookAt matrix
		ViewSetup& getViewSetup();
		PerspectiveSetup& getPerspectiveSetup();
		void rotate_FPS_style(glm::vec2 delta);
		glm::mat4 setCamera();
		glm::mat4 getProjection();
		bool checkFrustum(glm::vec3 pos, float radius);
		void updateAspectRatio(float width, float height);
		void moveCameraForeward();
		void moveCameraLeft();
		void moveCameraRight();
		void moveCameraBack();
		void stopCameraForeward();
		void stopCameraLeft();
		void stopCameraRight();
		void stopCameraBack();
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
