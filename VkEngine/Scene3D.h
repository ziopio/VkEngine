#pragma once
#include "Object3D.h"
#include "LightSource.h"
#include "Camera.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace vkengine
{

	class Scene3D
	{
	public:
		Scene3D(std::string id, std::string name);
		inline std::string getId() { return id; };
		void addCamera(std::string name, ViewSetup view, PerspectiveSetup perspective);
		Camera* getCamera(unsigned);
		std::vector<unsigned> listCameras();

		void addObject(vkengine::ObjectInitInfo obj_info);
		Object3D* getObject(unsigned id);
		std::vector<unsigned> listObjects();
		void removeObject(unsigned id);
		inline unsigned get_object_num() { return objects.size(); }

		void addLight(vkengine::PointLightInfo);
		PointLight* getLight(unsigned id);
		std::vector<unsigned> listLights();
		void removeLight(unsigned id);
		std::vector<SceneElement*> getAllElements();
		/*
		* Used by the engine to manage allocation depending on the instance count in the scene.
		* The capacity is managed by the Scene3D to always be >= then the number of objects.
		*/
		inline unsigned getCurrentObjectCapacity() { return object_capacity; };

		~Scene3D();
	public:
		unsigned current_camera;
		std::string name;
		LightData globalLight;
		Camera default_camera;
	private:
		std::string id;
		unsigned object_capacity;
		std::unordered_map<unsigned, Camera> cameras;
		std::unordered_map<unsigned, Object3D> objects;
		std::unordered_map<unsigned, PointLight> point_lights;

	};
}

