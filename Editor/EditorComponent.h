#pragma once
class EditorUI;

class EditorComponent
{
protected:
	const char* name;
	bool visible;
	EditorUI* UI;
public:
	EditorComponent(EditorUI* UI) { this->UI = UI; };
	const char* getName() { return name; }
	bool isVisible() { return visible; }
	void hide() { visible = false; }
	void show() { visible = true; }
	virtual void draw() = 0;
};