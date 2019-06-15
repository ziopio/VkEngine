#pragma once
#include <string>
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
	void updateKeyboard(int key, int scancode, int action, int mods);
	void updateChar(unsigned int character);
	void updateScroll(double xoffset, double yoffset);
	void updateCursor();
	void setDeltaTime(double delta_time);
	void showDebugString(std::string debug_info);
	FontAtlas getDefaultFontAtlas();
	inline bool wantCaptureMouse() { return _wantCaptureMouse; };
	inline bool wantCaptureKeyboard() { return _wantCaptureKeyboard; };
	inline bool wantTextInput() { return _wantTextInput; };
	inline bool wantSetMousePos() {	return _wantSetMousePos;};
private:
	void mapWindowInput2ImGui();
	void pollInputs();
	void setCaptureFlags();
	Editor* editor;
	bool mouseButtonsHasBeenPressed[5];
	bool _wantCaptureMouse;   
	bool _wantCaptureKeyboard;
	bool _wantTextInput;      
	bool _wantSetMousePos;
};
