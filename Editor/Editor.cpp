#include "..\\VkEngine\VkEngine.h"
#include "Editor.h"
#include "EditorUI.h"
#include "Project.h"
#include <iostream>

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
		vkengine::unified_delta_time = delta_time;
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
	this->loadedProject->save();
	vkengine::shutdown();
	delete UI;
	WindowManager::terminate();
}

void Editor::load_project(const char* project_dir)
{
	this->loadedProject = std::make_unique<Project>(project_dir);
	this->loadedProject->load();
}

void window_system_debug_callback(int error, const char * description)
{
	std::cout << "WindowManager  ERROR " << error << ": " << description << std::endl;
}
