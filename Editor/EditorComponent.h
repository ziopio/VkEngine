#pragma once
#include "../ImGui/imgui.h"
class EditorUI;

class EditorComponent
{
protected:
	const char* name;
	bool visible;
	EditorUI* UI;
public:
	EditorComponent(EditorUI* UI) { this->UI = UI; };
	inline const char* getName() { return name; }
	inline bool isVisible() { return visible; }
	inline void hide() { visible = false; }
	inline void show() { visible = true; }
	virtual void draw(int w_width, int w_height) = 0;
};