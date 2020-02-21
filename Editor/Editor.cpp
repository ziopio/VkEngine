#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Editor.h"
#include "EditorUI.h"
#include <iostream>
#include <fstream>
// nlohmann/json.hpp
#include "json.hpp"
// for convenience
using json = nlohmann::json;

constexpr const char* default_proj = "Data/default_project/";

void window_system_debug_callback(int error, const char *description);

Editor::Editor() 
{
	WindowManager::init();
	WindowManager::setDebugCallBack(window_system_debug_callback);
	this->UI = new EditorUI(this);
	try {
		vkengine::setSurfaceOwner(this->UI);
		vkengine::init();
		FontAtlas f = UI->getDefaultFontAtlas();
		vkengine::loadFontAtlas(f.pixels, &f.width, &f.height);
		this->load_project(default_proj);
	}
	catch (std::runtime_error err){
		std::cout << "Engine Initialization FAILED: " << err.what() << std::endl;
		this->terminate = true;
	}
}

void Editor::execute()
{
	static double last_iteration;
	double delta_time;
	double now;
	while (!this->UI->getWindow()->windowShouldClose() || this->terminate) 
	{
		WindowManager::pollEvents();
		now = WindowManager::getTime();
		delta_time = now - last_iteration;
		this->UI->setDeltaTime(delta_time);
		last_iteration = now;

		vkengine::updateImGuiData(this->UI->drawUI());
		vkengine::renderFrame();
	}
}

void Editor::resizeSwapChain()
{
	vkengine::resizeSwapchain();
}

Editor::~Editor()
{
	vkengine::shutdown();
	delete UI;
	WindowManager::terminate();
}

void Editor::load_project(const char* project_dir)
{
	json project;

	std::ifstream i((std::string(project_dir) + "proj_config.json").c_str());
	i >> project;
	this->UI->printDebug("Project JSON: \n" + project.dump());

	vkengine::loadMesh("VkEngine/Meshes/cube.obj");
	//this->renderingEngine->loadMesh("VkEngine/Meshes/icosphere.obj");
	//this->renderingEngine->loadMesh("VkEngine/Meshes/sphere.obj");
	vkengine::loadTexture("VkEngine/Textures/cubeTex.png");
	//this->renderingEngine->loadTexture("VkEngine/Textures/AXIS_TEX.png");
	//this->renderingEngine->loadTexture("VkEngine/Textures/.png");

	vkengine::PointLightInfo l = {
		3,3,3,
		1,1,1,
		2
	};
	vkengine::addLight(l);
	{
		float position[] = { 0,0,0 };
		float rotation_vector[] = { -1,1,-1 };
		float scale_vector[] = { 1,1,1 };
		vkengine::ObjTransformation  t = {};
		t.angularSpeed = 30.0f;
		std::copy(std::begin(position), std::end(position), std::begin(t.position));
		std::copy(std::begin(rotation_vector), std::end(rotation_vector), std::begin(t.rotation_vector));
		t.scale_factor = 1.;
		std::copy(std::begin(scale_vector), std::end(scale_vector), std::begin(t.scale_vector));

		vkengine::ObjectInitInfo cube = {};
		cube.mesh_id = 0;
		cube.texture_id = 1;
		cube.material_id = 1;
		cube.transformation = t;
		vkengine::addObject(cube);
	}

}

void window_system_debug_callback(int error, const char * description)
{
	std::cout << "WindowManager  ERROR " << error << ": " << description << std::endl;
}
