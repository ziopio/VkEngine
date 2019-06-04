#pragma once
#include "WindowInputEnums.h"

class WindowEventHandler {
public:
	virtual void onWindowResizeCallBack(int width, int height) = 0;
	virtual void onKeyCallBack(KeyType key, int scancode, ActionType action, ModifierKeyType mods) = 0;
	virtual void onCharCallback(unsigned int code_point) = 0;
	virtual void onCursorPosCallback(double xpos, double ypos) = 0;
	virtual void onMouseButtonCallback(MouseButtonType button, ActionType action, ModifierKeyType mods) = 0;
	virtual void onScrollCallback(double xoffset, double yoffset) = 0;
	virtual void onDropCallback(int count, const char** paths) = 0;
};

class WindowManager
{
public:
	class Window;
	static void init();
	static const char** getRequiredInstanceExtensions4Vulkan(unsigned int *extension_count);
	static void setDebugCallBack(void(*func)(int error, const char* description));
	static WindowManager::Window* createWindow( unsigned int width, unsigned int height, const char* title);
	static void destroyWindow(Window* window);
	static void pollEvents();
	static void waitEvents();
	static void terminate();
	class Window
	{
		friend class WindowManager;
	private:
		WindowEventHandler* window_client;
		struct SurfaceCreationInfo;
		struct _window_impl;
		_window_impl* pimpl;
		Window( unsigned int width, unsigned int height, const char* title);
		~Window();
	public:
		void registerEventHandler(WindowEventHandler* handler);
		bool windowShouldClose();
		void createWindowSurface(void* instance, void *surface);
		void getFrameBufferSize(int *width, int *height);
		void getCursorPos(double * xpos, double * ypos);
		void getMouseButton(int button);
		void activateKeyCallBack();
		void activateCharCallback();
		void activateCursorPosCallback();
		void activateMouseButtonCallback();
		void activateScrollCallback();
		void activateDropCallback();
	};
};


