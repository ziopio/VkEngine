#include "Editor.h"
#include <iostream>
#include "glm/glm.hpp"

#define W_WIDTH 500
#define W_HEIGHT 500

Editor::Editor() 
{
	WindowManager::init();
	this->window = WindowManager::createWindow( W_WIDTH, W_HEIGHT, "Editor!!!");
	this->window->registerEventHandler(this);
	//this->window->activateKeyCallBack();
	//this->window->activateMouseButtonCallback();
	//this->window->activateCursorPosCallback();
	//this->window->activateScrollCallback();
	this->renderingEngine.setSurfaceOwner(this);
	this->renderingEngine.init();
	this->load_demo_scene();
}

void Editor::execute()
{
	while (!this->window->windowShouldClose() || this->terminate) {
		WindowManager::pollEvents();
		renderingEngine.renderFrame();
	}
}

Editor::~Editor()
{
	WindowManager::destroyWindow(this->window);
	WindowManager::terminate();
}

void Editor::onWindowResizeCallBack(int width, int height)
{	
	this->renderingEngine.resizeSwapchain(width, height);
}

void Editor::onKeyCallBack(KeyType key, int scancode, ActionType action, ModifierKeyType mods)
{
	std::cout << "Key callback" << std::endl;
}

void Editor::onCharCallback(unsigned int code_point)
{
	std::cout << "Char callback" << std::endl;
}

void Editor::onCursorPosCallback(double xpos, double ypos)
{
	std::cout << "Cursor pos callback" << std::endl;
}

void Editor::onMouseButtonCallback(MouseButtonType button, ActionType action, ModifierKeyType mods)
{
	std::cout << "Key callback" << std::endl;
}

void Editor::onScrollCallback(double xoffset, double yoffset)
{
	std::cout << "Scroll callback" << std::endl;
}

void Editor::onDropCallback(int count, const char ** paths)
{
	std::cout << "Drop file callback" << std::endl;
}

VulkanInstanceInitInfo Editor::getInstanceExtInfo()
{
	VulkanInstanceInitInfo info = {};
	info.instanceExtensions = WindowManager::getRequiredInstanceExtensions4Vulkan(&info.instance_extension_count);
	info.enableValidation = true;
	return info;
}

void * Editor::getSurface(void * vulkan_instance)
{
	void* surface;
	this->window->createWindowSurface(vulkan_instance, &surface);
	return surface;
}

void Editor::getFrameBufferSize(int * width, int * height)
{
	this->window->getFrameBufferSize(width, height);
}

void Editor::waitEvents()
{
	WindowManager::waitEvents();
}

void Editor::load_demo_scene()
{
	this->renderingEngine.loadTexture("VkEngine/Textures/cube1.png");
	PointLightInfo l = {
		2,2,2,
		1,1,1,
		1
	};
	this->renderingEngine.addLight(l);

	float position[] = { 0,0,0 };
	float rotation_vector[] = { 1,1,1 };
	float scale_vector[] = { 1,1,1 };
	ObjTransformation  t = {};
	t.angularSpeed = 30.0f;
	std::copy(std::begin(position), std::end(position), std::begin(t.position));
	std::copy(std::begin(rotation_vector), std::end(rotation_vector),std::begin(t.rotation_vector));
	t.scale_factor = 1.;
	std::copy(std::begin(scale_vector), std::end(scale_vector), std::begin(t.scale_vector));

	ObjectInitInfo cube = {};
	cube.mesh_id = 0;
	cube.texture_id = 1;
	cube.material_id = 1;
	cube.transformation = t;
	this->renderingEngine.addObject(cube);
}

