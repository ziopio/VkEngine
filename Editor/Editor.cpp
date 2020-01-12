#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Editor.h"
#include "EditorUI.h"

void window_system_debug_callback(int error, const char *description);

Editor::Editor() 
{
	WindowManager::init();
	WindowManager::setDebugCallBack(window_system_debug_callback);
	this->UI = new EditorUI(this);
	this->renderingEngine = new VkEngine();
	try {
		this->renderingEngine->setSurfaceOwner(this->UI);
		this->renderingEngine->init();
		FontAtlas f = UI->getDefaultFontAtlas();
		this->renderingEngine->loadFontAtlas(f.pixels, &f.width, &f.height);
		this->load_demo_scene();
	}
	catch (std::runtime_error err){
		std::cout << err.what() << std::endl;
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

		renderingEngine->updateImGuiData(this->UI->drawUI());
		renderingEngine->renderFrame();
	}
}

void Editor::resizeSwapChain()
{
	this->renderingEngine->resizeSwapchain();
}

Editor::~Editor()
{
	delete renderingEngine;
	delete UI;
	WindowManager::terminate();
}

void Editor::load_demo_scene()
{

	this->renderingEngine->loadMesh("VkEngine/Meshes/cube.obj");
	//this->renderingEngine->loadMesh("VkEngine/Meshes/icosphere.obj");
	//this->renderingEngine->loadMesh("VkEngine/Meshes/sphere.obj");
	this->renderingEngine->loadTexture("VkEngine/Textures/cubeTex.png");
	//this->renderingEngine->loadTexture("VkEngine/Textures/AXIS_TEX.png");
	//this->renderingEngine->loadTexture("VkEngine/Textures/.png");

	PointLightInfo l = {
		3,3,3,
		1,1,1,
		2
	};
	this->renderingEngine->addLight(l);
	{
		float position[] = { 0,0,0 };
		float rotation_vector[] = { -1,1,-1 };
		float scale_vector[] = { 1,1,1 };
		ObjTransformation  t = {};
		t.angularSpeed = 30.0f;
		std::copy(std::begin(position), std::end(position), std::begin(t.position));
		std::copy(std::begin(rotation_vector), std::end(rotation_vector), std::begin(t.rotation_vector));
		t.scale_factor = 1.;
		std::copy(std::begin(scale_vector), std::end(scale_vector), std::begin(t.scale_vector));

		ObjectInitInfo cube = {};
		cube.mesh_id = 0;
		cube.texture_id = 1;
		cube.material_id = 1;
		cube.transformation = t;
		this->renderingEngine->addObject(cube);
	}
	//{
	//	float position[] = { 0,0,0 };
	//	float rotation_vector[] = { 1,1,1 };
	//	float scale_vector[] = { 1,1,1 };
	//	ObjTransformation  t = {};
	//	t.angularSpeed = 0.0f;
	//	std::copy(std::begin(position), std::end(position), std::begin(t.position));
	//	std::copy(std::begin(rotation_vector), std::end(rotation_vector), std::begin(t.rotation_vector));
	//	t.scale_factor = 1.;
	//	std::copy(std::begin(scale_vector), std::end(scale_vector), std::begin(t.scale_vector));

	//	ObjectInitInfo axis = {};
	//	axis.mesh_id = 1;
	//	axis.texture_id = 2;
	//	axis.material_id = 0;
	//	axis.transformation = t;
	//	this->renderingEngine->addObject(axis);
	//}

}

void window_system_debug_callback(int error, const char * description)
{
	std::cout << "WindowManager  ERROR " << error << ": " << description << std::endl;
}
