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
	void  load_project(const char* project_dir);
	EditorUI* UI;
	bool terminate;
};

