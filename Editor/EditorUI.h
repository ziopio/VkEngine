#pragma once
#include "..\\Platform\WindowManager.h"
#include "..\\VkEngine\VkEngine.h"
#include <sstream>
#include <vector>
#include <deque>
#define VIEW_3D_TEXTURE_CODE -1 // special case in gui fragment shader

class Editor;
class EditorComponent;

typedef struct {
	unsigned char* pixels;
	int width;
	int height;
} FontAtlas;

constexpr const char* win_title = "Editor";

class EditorUI : WindowEventHandler, public vkengine::SurfaceOwner
{
public:
	EditorUI(Editor* editor);
	FontAtlas getDefaultFontAtlas();
	void setDeltaTime(double delta_time);
	vkengine::UiDrawData drawUI();
	~EditorUI();
	// WindowEventHandler
	inline WindowManager::Window* getWindow() { return this->window; }
	inline bool wantCaptureMouse() { return _wantCaptureMouse; };
	inline bool wantCaptureKeyboard() { return _wantCaptureKeyboard; };
	inline bool wantTextInput() { return _wantTextInput; };
	inline bool wantSetMousePos() {	return _wantSetMousePos;};
	// Ereditato tramite SurfaceOwner
	vkengine::VulkanInstanceInitInfo getInstanceExtInfo() override;
	void *getSurface(void * vulkan_instance) override;
	void getFrameBufferSize(int * width, int * height) override;
	virtual void printDebug(std::string msg) override;
	void waitEvents() override;
private:
	void mapWindowInput2ImGui();
	void pollInputs();
	void updateCursor();
	void updateImguiDisplay();
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
	std::vector<EditorComponent*> editorComponents;
	WindowManager::Window* window;
	bool mouseButtonsHaveBeenPressed[5];
	bool _wantCaptureMouse;   
	bool _wantCaptureKeyboard;
	bool _wantTextInput;      
	bool _wantSetMousePos;
	std::deque<std::string> debug_logs;
};
