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
	virtual void onFrameBufferResizeCallBack(int width, int height) override;
	virtual void onKeyCallBack(KeyType key, int scancode, ActionType action, ModifierKeyType mods) override;
	virtual void onCharCallback(unsigned int code_point) override;
	virtual void onCursorPosCallback(double xpos, double ypos) override;
	virtual void onMouseButtonCallback(MouseButtonType button, ActionType action, ModifierKeyType mods) override;
	virtual void onScrollCallback(double xoffset, double yoffset) override;
	virtual void onDropCallback(int count, const char ** paths) override;
	// Ereditato tramite SurfaceOwner
	virtual VulkanInstanceInitInfo getInstanceExtInfo() override;
	virtual void *getSurface(void * vulkan_instance) override;
	virtual void getFrameBufferSize(int * width, int * height) override;
	virtual void waitEvents() override;
private:
	WindowManager::Window* window;
	EditorUI* UI;
	VkEngine renderingEngine;
	bool terminate;
	void load_demo_scene();




};

