#pragma once
#include "..\\VkEngine\VkEngine.h"
#include "..\\Platform\WindowManager.h"

class EditorUI;



class Editor : WindowEventHandler, SurfaceOwner
{
public:
	Editor();
	void execute();
	~Editor();
	WindowManager::Window* getWindow();
	// Ereditato tramite WindowEventHandler
	void onFrameBufferResizeCallBack(int width, int height) override;
	void onKeyCallBack(KeyType key, int scancode, ActionType action, 
		ModifierKeyType mods) override;
	void onCharCallback(unsigned int code_point) override;
	void onCursorPosCallback(double xpos, double ypos) override;
	void onMouseButtonCallback(MouseButtonType button, ActionType action,
		ModifierKeyType mods) override;
	void onScrollCallback(double xoffset, double yoffset) override;
	void onDropCallback(int count, const char ** paths) override;
	// Ereditato tramite SurfaceOwner
	VulkanInstanceInitInfo getInstanceExtInfo() override;
	void *getSurface(void * vulkan_instance) override;
	void getFrameBufferSize(int * width, int * height) override;
	void waitEvents() override;
private:
	WindowManager::Window* window;
	EditorUI* UI;
	VkEngine renderingEngine;
	bool terminate;
	void load_demo_scene();
};

