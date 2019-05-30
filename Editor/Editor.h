#pragma once
#include "..\\VkEngine\VkEngine.h"
#include "..\\Platform\WindowManager.h"


class Editor : WindowEventHandler
{
private:
	WindowManager::Window* window;
	VkEngine renderingEngine;
	bool terminate;
public:
	Editor();
	void execute();
	~Editor();

	// Ereditato tramite WindowEventHandler
	virtual void onKeyCallBack(KeyType key, int scancode, ActionType action, ModifierKeyType mods) override;
	virtual void onCharCallback(unsigned int code_point) override;
	virtual void onCursorPosCallback(double xpos, double ypos) override;
	virtual void onMouseButtonCallback(MouseButtonType button, ActionType action, ModifierKeyType mods) override;
	virtual void onScrollCallback(double xoffset, double yoffset) override;
	virtual void onDropCallback(int count, const char ** paths) override;
};

