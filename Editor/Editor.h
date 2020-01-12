#pragma once

class VkEngine;
class EditorUI;

class Editor
{
public:
	Editor();
	void execute();
	void resizeSwapChain();
	~Editor();
private:
	void load_demo_scene();
	EditorUI* UI;
	VkEngine* renderingEngine;
	bool terminate;
};

