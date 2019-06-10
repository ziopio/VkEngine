#pragma once

class Editor;
struct UiDrawData;

typedef struct {
	unsigned char* pixels;
	int width;
	int height;
} FontAtlas;

class EditorUI
{
public:
	EditorUI(Editor* editor);
	UiDrawData drawUI();
	~EditorUI();
	void updateFrameSize(int w_width, int w_height, int frame_width, int frame_height);
	void updateMousePos(double xpos, double ypos);
	void updateMouseButton(int button, int action, int mods);
	void updateCursor();
	FontAtlas getDefaultFontAtlas();
private:
	void mapWindowInput2ImGui();
	Editor* editor;
};
