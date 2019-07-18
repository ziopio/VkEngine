#pragma once
#include "..\\Platform\WindowManager.h"
#include "..\\VkEngine\VkEngine.h"
#include <sstream>

class Editor;

typedef struct {
	unsigned char* pixels;
	int width;
	int height;
} FontAtlas;

class EditorUI : WindowEventHandler, public SurfaceOwner
{
public:
	EditorUI(Editor* editor);
	FontAtlas getDefaultFontAtlas();
	void setDeltaTime(double delta_time);
	UiDrawData drawUI();
	~EditorUI();
	inline WindowManager::Window* getWindow() { return this->window; }
	inline bool wantCaptureMouse() { return _wantCaptureMouse; };
	inline bool wantCaptureKeyboard() { return _wantCaptureKeyboard; };
	inline bool wantTextInput() { return _wantTextInput; };
	inline bool wantSetMousePos() {	return _wantSetMousePos;};
	// Ereditato tramite SurfaceOwner
	VulkanInstanceInitInfo getInstanceExtInfo() override;
	void *getSurface(void * vulkan_instance) override;
	void getFrameBufferSize(int * width, int * height) override;
	virtual void printDebug(std::string msg) override;
	void waitEvents() override;
private:
	void mapWindowInput2ImGui();
	void pollInputs();
	void updateCursor();
	void updateImguiDisplay(int width, int height);
	void setCaptureFlags();
	// Ereditato tramite WindowEventHandler
	void onFrameBufferResizeCallBack(int width, int height) override;
	void onKeyCallBack(KeyType key, int scancode, ActionType action,
		ModifierKeyType mods) override;
	void onCharCallback(unsigned int code_point) override;
	void onCursorPosCallback(double xpos, double ypos) override;
	void onMouseButtonCallback(MouseButtonType button, ActionType action,
		ModifierKeyType mods) override;
	void onScrollCallback(double xoffset, double yoffset) override;
	void onDropCallback(int count, const char ** paths) override;
	void setUpImGuiStyle();
	Editor* editor;
	WindowManager::Window* window;
	bool mouseButtonsHaveBeenPressed[5];
	bool _wantCaptureMouse;   
	bool _wantCaptureKeyboard;
	bool _wantTextInput;      
	bool _wantSetMousePos;
	std::vector<std::string> debug_logs;
};
