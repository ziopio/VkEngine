#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include "Object3D.h"
#include "LightSource.h"
#include "Camera.h"
#include <string>
#include <unordered_map>

namespace vkengine
{

	class Scene3D
	{
	public:
		Scene3D(std::string id);

		void addCamera(std::string id, std::string name, ViewSetup view, PerspectiveSetup perspective);
		Camera* getCamera(std::string id);
		std::vector<std::string> listCameras();

		void addObject(vkengine::ObjectInitInfo obj_info);
		Object3D* getObject(std::string id);
		std::vector<std::string> listObjects();
		inline unsigned get_object_num() { return objects.size(); }
		void removeObject(std::string id);

		void addLight(vkengine::PointLightInfo);
		LightSource* getLight(std::string id);		
		std::vector<std::string> listLights();
		void removeLight(std::string id);

		std::vector<SceneElement*> getAllElements();

		~Scene3D();
	public:
		std::string current_camera;
	private:
		std::string id;
		std::unordered_map<std::string, Camera> cameras;
		std::unordered_map<std::string, Object3D> objects;
		std::unordered_map<std::string, LightSource> lights;
	};
}

