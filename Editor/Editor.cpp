#include "Editor.h"
#include <iostream>

#define W_WIDTH 500
#define W_HEIGHT 500

Editor::Editor() 
{
	WindowManager::init();
	this->window = WindowManager::createWindow( W_WIDTH, W_HEIGHT, "Editor!!!");
	this->window->registerEventHandler(this);
	this->window->activateKeyCallBack();
	this->window->activateMouseButtonCallback();
	this->window->activateCursorPosCallback();
	this->window->activateScrollCallback();
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

}

