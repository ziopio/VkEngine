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
	bool terminate;
std::unique_ptr<Project> loadedProject;
private:
	void load_project(const char* project_dir);
	EditorUI* UI;
};

