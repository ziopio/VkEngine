#pragma once
#include <memory>

class VkEngine;
class EditorUI;
class Project;

class Editor
{
public:
	Editor();
	void execute();
	void resizeSwapChain();
	~Editor();
private:
	void load_project(const char* project_dir);
	std::unique_ptr<Project> loadedProject;
	EditorUI* UI;
	bool terminate;
};

