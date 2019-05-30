#include "WindowManager.h"
#include <iostream>
#include <assert.h>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


// internal glfw callbacks

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void charCallback(GLFWwindow* window, unsigned int code_point);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void dropCallback(GLFWwindow* window, int count, const char** paths);

void WindowManager::init()
{
	if (!glfwInit()) 
	{
		std::cout << "GLFW: glfwInit FAILED" << std::endl;
	}
	if (!glfwVulkanSupported())
	{
		std::cout << "GLFW: Vulkan Not Supported" << std::endl;
	}
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
}

void WindowManager::setDebugCallBack(void(*func)(int error, const char *description))
{
	glfwSetErrorCallback(func);
}

WindowManager::Window* WindowManager::createWindow( unsigned int width, unsigned int height, const char * title)
{
	return new Window( width, height, title);
}

void WindowManager::destroyWindow(Window * window)
{
	delete window;
}

void WindowManager::pollEvents()
{
	glfwPollEvents();
}

void WindowManager::waitEvents()
{
	glfwWaitEvents();
}

void WindowManager::terminate()
{
	glfwTerminate();
}

//**************** Window implementation*********************//

// Impl struct definition pointed by the pimpl
struct WindowManager::Window::_window_impl {
	GLFWwindow* window;
};

struct WindowManager::Window::SurfaceCreationInfo {
	VkInstance instance;
	VkSurfaceKHR *surface;
};

WindowManager::Window::Window( unsigned int width, unsigned int height, const char* title)
	: pimpl(new _window_impl())
{
	this->pimpl->window = glfwCreateWindow(width, height, title, NULL, NULL);
}

WindowManager::Window::~Window()
{
	glfwDestroyWindow(this->pimpl->window);
	delete pimpl;
}

void WindowManager::Window::registerEventHandler(WindowEventHandler * handler)
{
	this->window_client = handler;
	glfwSetWindowUserPointer(this->pimpl->window, window_client);
}

bool WindowManager::Window::windowShouldClose()
{
	return glfwWindowShouldClose(this->pimpl->window);
}

void WindowManager::Window::createWindowSurface(SurfaceCreationInfo *info)
{
	if (glfwCreateWindowSurface(info->instance, this->pimpl->window, nullptr, info->surface)) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void WindowManager::Window::getFrameBufferSize(int * width, int * height)
{
	glfwGetFramebufferSize(this->pimpl->window, width, height);
}

void WindowManager::Window::getCursorPos(double * xpos, double * ypos)
{
	glfwGetCursorPos(this->pimpl->window, xpos, ypos);
}

void WindowManager::Window::getMouseButton(int button)
{
	glfwGetMouseButton(this->pimpl->window, button);
}

void WindowManager::Window::activateKeyCallBack()
{
	glfwSetKeyCallback(this->pimpl->window, keyCallBack);
}

void WindowManager::Window::activateCharCallback()
{
	glfwSetCharCallback(this->pimpl->window, charCallback);
}

void WindowManager::Window::activateCursorPosCallback()
{
	glfwSetCursorPosCallback(this->pimpl->window, cursorPosCallback);
}

void WindowManager::Window::activateMouseButtonCallback()
{
	glfwSetMouseButtonCallback(this->pimpl->window, mouseButtonCallback);
}

void WindowManager::Window::activateScrollCallback()
{
	glfwSetScrollCallback(this->pimpl->window, scrollCallback);
}

void WindowManager::Window::activateDropCallback()
{
	glfwSetDropCallback(this->pimpl->window, dropCallback);
}

/* GLFW callbacks to map GLFW types to the outside */


void charCallback(GLFWwindow * window, unsigned int code_point)
{
	auto client = static_cast<WindowEventHandler*>(glfwGetWindowUserPointer(window));
	client->onCharCallback(code_point);
}

void cursorPosCallback(GLFWwindow * window, double xpos, double ypos)
{
	auto client = static_cast<WindowEventHandler*>(glfwGetWindowUserPointer(window));
	client->onCursorPosCallback(xpos,ypos);
}

void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
	auto client = static_cast<WindowEventHandler*>(glfwGetWindowUserPointer(window));
	MouseButtonType t;
	ActionType a;
	ModifierKeyType m;
	switch (action)
	{
	case GLFW_PRESS: a = ActionType::PRESS;	break;
	case GLFW_RELEASE: a = ActionType::RELEASE;	break;
	case GLFW_REPEAT: a = ActionType::REPEAT; break;
	default: a = ActionType::NONE;
		break;
	}

	switch (mods)
	{
	case GLFW_MOD_SHIFT: m = ModifierKeyType::MODIFIER_SHIFT;	break;
	case GLFW_MOD_CONTROL: m = ModifierKeyType::MODIFIER_CTRL;	break;
	case GLFW_MOD_ALT: m = ModifierKeyType::MODIFIER_ALT;	break;
	case GLFW_MOD_SUPER: m = ModifierKeyType::MODIFIER_SUPER;	break;
	case GLFW_MOD_CAPS_LOCK: m = ModifierKeyType::MODIFIER_CAPS_LOCK;	break;
	case GLFW_MOD_NUM_LOCK: m = ModifierKeyType::MODIFIER_NUM_LOCK;	break;
	default: m = ModifierKeyType::MODIFIER_NONE;break;
	}

	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LAST: t = MouseButtonType::MOUSE_BUTTON_LAST; break;
	case GLFW_MOUSE_BUTTON_LEFT: t = MouseButtonType::MOUSE_BUTTON_LEFT; break;
	case GLFW_MOUSE_BUTTON_RIGHT: t = MouseButtonType::MOUSE_BUTTON_RIGHT; break;
	case GLFW_MOUSE_BUTTON_MIDDLE: t = MouseButtonType::MOUSE_BUTTON_MIDDLE; break;

	case GLFW_MOUSE_BUTTON_4: t = MouseButtonType::MOUSE_BUTTON_4; break;
	case GLFW_MOUSE_BUTTON_5: t = MouseButtonType::MOUSE_BUTTON_5; break;
	case GLFW_MOUSE_BUTTON_6: t = MouseButtonType::MOUSE_BUTTON_6; break;
	case GLFW_MOUSE_BUTTON_7: t = MouseButtonType::MOUSE_BUTTON_7; break;
	//case GLFW_MOUSE_BUTTON_8: t = MouseButtonType::MOUSE_BUTTON_8;break;
	//case GLFW_MOUSE_BUTTON_1: t = MouseButtonType::MOUSE_BUTTON_1; break;
	//case GLFW_MOUSE_BUTTON_2: t = MouseButtonType::MOUSE_BUTTON_2; break;
	//case GLFW_MOUSE_BUTTON_3: t = MouseButtonType::MOUSE_BUTTON_3; break;

	default: t = MouseButtonType::MOUSE_BUTTON_NONE; break;
	}
	client->onMouseButtonCallback(t, a, m);
}

void scrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
	auto client = static_cast<WindowEventHandler*>(glfwGetWindowUserPointer(window));
	client->onScrollCallback(xoffset,yoffset);
}

void dropCallback(GLFWwindow * window, int count, const char ** paths)
{
	auto client = static_cast<WindowEventHandler*>(glfwGetWindowUserPointer(window));
	client->onDropCallback(count, paths);
}

void keyCallBack(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	auto client = static_cast<WindowEventHandler*>(glfwGetWindowUserPointer(window));
	KeyType t;
	ActionType a;
	ModifierKeyType m;
	switch (action)
	{
	case GLFW_PRESS: a = ActionType::PRESS;	break;
	case GLFW_RELEASE: a = ActionType::RELEASE;	break;
	case GLFW_REPEAT: a = ActionType::REPEAT; break;
	default: a = ActionType::NONE;
		break;
	}

	switch (mods)
	{
	case GLFW_MOD_SHIFT: m = ModifierKeyType::MODIFIER_SHIFT;break;
	case GLFW_MOD_CONTROL: m = ModifierKeyType::MODIFIER_CTRL;break;
	case GLFW_MOD_ALT: m = ModifierKeyType::MODIFIER_ALT;break;
	case GLFW_MOD_SUPER: m = ModifierKeyType::MODIFIER_SUPER;break;
	case GLFW_MOD_CAPS_LOCK: m = ModifierKeyType::MODIFIER_CAPS_LOCK;break;
	case GLFW_MOD_NUM_LOCK: m = ModifierKeyType::MODIFIER_NUM_LOCK;break;
	default: m = ModifierKeyType::MODIFIER_NONE;break;
	}

	switch (key)
	{
	case GLFW_KEY_SPACE: t = KeyType::KEY_ESCAPE; break;
	case GLFW_KEY_APOSTROPHE:	t = KeyType::KEY_APOSTROPHE ;break;					
	case GLFW_KEY_COMMA:		t = KeyType::KEY_COMMA ;break;					
	case GLFW_KEY_MINUS:	t = KeyType::KEY_MINUS;break;
	case GLFW_KEY_PERIOD:	t = KeyType::KEY_PERIOD;break;
	case GLFW_KEY_SLASH:	t = KeyType::KEY_SLASH;break;
	case GLFW_KEY_0:		t = KeyType::KEY_0 ;break;				
	case GLFW_KEY_1:		t = KeyType::KEY_1;break;
	case GLFW_KEY_2:		t = KeyType::KEY_2;break;
	case GLFW_KEY_3:		t = KeyType::KEY_3;break;
	case GLFW_KEY_4:		t = KeyType::KEY_4 ;break;				
	case GLFW_KEY_5:		t = KeyType::KEY_5;break;
	case GLFW_KEY_6:		t = KeyType::KEY_6;break;
	case GLFW_KEY_7:		t = KeyType::KEY_7;break;
	case GLFW_KEY_8:		t = KeyType::KEY_8;break;
	case GLFW_KEY_9:		t = KeyType::KEY_9;break;
	case GLFW_KEY_SEMICOLON:t = KeyType::KEY_SEMICOLON;break;
	case GLFW_KEY_EQUAL:	t = KeyType::KEY_EQUAL;break;
	case GLFW_KEY_A:		t = KeyType::KEY_A ;break;				
	case GLFW_KEY_B:		t = KeyType::KEY_B;break;
	case GLFW_KEY_C:		t = KeyType::KEY_C;break;
	case GLFW_KEY_D:		t = KeyType::KEY_D;break;
	case GLFW_KEY_E:		t = KeyType::KEY_E;break;
	case GLFW_KEY_F:		t = KeyType::KEY_F;break;
	case GLFW_KEY_G:		t = KeyType::KEY_G;break;
	case GLFW_KEY_H:		t = KeyType::KEY_H;break;
	case GLFW_KEY_I:		t = KeyType::KEY_I ;break;				
	case GLFW_KEY_J:		t = KeyType::KEY_J;break;
	case GLFW_KEY_K:		t = KeyType::KEY_K;break;
	case GLFW_KEY_L:		t = KeyType::KEY_L;break;
	case GLFW_KEY_M:		t = KeyType::KEY_M;break;
	case GLFW_KEY_N:		t = KeyType::KEY_N;break;
	case GLFW_KEY_O:		t = KeyType::KEY_O;break;
	case GLFW_KEY_P:		t = KeyType::KEY_P;break;
	case GLFW_KEY_Q:		t = KeyType::KEY_Q ;break;					
	case GLFW_KEY_R:		t = KeyType::KEY_R;break;
	case GLFW_KEY_S:		t = KeyType::KEY_S;break;
	case GLFW_KEY_T:		t = KeyType::KEY_T;break;
	case GLFW_KEY_U:		t = KeyType::KEY_U;break;
	case GLFW_KEY_V:		t = KeyType::KEY_V;break;
	case GLFW_KEY_W:		t = KeyType::KEY_W;break;
	case GLFW_KEY_X:		t = KeyType::KEY_X;break;
	case GLFW_KEY_Y:		t = KeyType::KEY_Y ;break;					
	case GLFW_KEY_Z:		t = KeyType::KEY_Z;break;
	case GLFW_KEY_LEFT_BRACKET:	 t = KeyType::KEY_LEFT_BRACKET;break;
	case GLFW_KEY_BACKSLASH: t = KeyType::KEY_BACKSLASH;break;
	case GLFW_KEY_RIGHT_BRACKET: t = KeyType::KEY_RIGHT_BRACKET;break;
	case GLFW_KEY_GRAVE_ACCENT: t = KeyType::KEY_GRAVE_ACCENT;break;
	case GLFW_KEY_WORLD_1:	t = KeyType::KEY_WORLD_1;break;
	case GLFW_KEY_WORLD_2:	t = KeyType::KEY_WORLD_2;break;
	case GLFW_KEY_ESCAPE:	t = KeyType::KEY_ESCAPE ;break;					
	case GLFW_KEY_ENTER:	t = KeyType::KEY_ENTER;break;
	case GLFW_KEY_TAB:		t = KeyType::KEY_TAB;break;
	case GLFW_KEY_BACKSPACE:t = KeyType::KEY_BACKSPACE;break;
	case GLFW_KEY_INSERT:	t = KeyType::KEY_INSERT;break;
	case GLFW_KEY_DELETE:	t = KeyType::KEY_DELETE;break;
	case GLFW_KEY_RIGHT:	t = KeyType::KEY_RIGHT;break;
	case GLFW_KEY_LEFT:		t = KeyType::KEY_LEFT;break;
	case GLFW_KEY_DOWN:		t = KeyType::KEY_DOWN ;break;					
	case GLFW_KEY_UP:		t = KeyType::KEY_UP;break;
	case GLFW_KEY_PAGE_UP:	t = KeyType::KEY_PAGE_UP;break;
	case GLFW_KEY_PAGE_DOWN:t = KeyType::KEY_PAGE_DOWN;break;
	case GLFW_KEY_HOME:		t = KeyType::KEY_HOME;break;
	case GLFW_KEY_END:		t = KeyType::KEY_END;break;
	case GLFW_KEY_CAPS_LOCK:t = KeyType::KEY_CAPS_LOCK;break;
	case GLFW_KEY_SCROLL_LOCK: t = KeyType::KEY_SCROLL_LOCK;break;
	case GLFW_KEY_NUM_LOCK:	t = KeyType::KEY_NUM_LOCK ;break;					
	case GLFW_KEY_PRINT_SCREEN: t =	KeyType::KEY_PRINT_SCREEN ;break;					
	case GLFW_KEY_PAUSE:	t = KeyType::KEY_PAUSE;break;
	case GLFW_KEY_F1:		t = KeyType::KEY_F1;break;
	case GLFW_KEY_F2:		t = KeyType::KEY_F2;break;
	case GLFW_KEY_F3:		t = KeyType::KEY_F3;break;
	case GLFW_KEY_F4:		t = KeyType::KEY_F4;break;
	case GLFW_KEY_F5:		t = KeyType::KEY_F5;break;
	case GLFW_KEY_F6:		t = KeyType::KEY_F6;break;
	case GLFW_KEY_F7:		t = KeyType::KEY_F7 ;break;					
	case GLFW_KEY_F8:		t = KeyType::KEY_F8;break;
	case GLFW_KEY_F9:		t = KeyType::KEY_F9;break;
	case GLFW_KEY_F10:		t = KeyType::KEY_F10;break;
	case GLFW_KEY_F11:		t = KeyType::KEY_F11;break;
	case GLFW_KEY_F12:		t = KeyType::KEY_F12;break;
	case GLFW_KEY_F13:		t = KeyType::KEY_F13;break;
	case GLFW_KEY_F14:		t = KeyType::KEY_F14;break;
	case GLFW_KEY_F15:		t = KeyType::KEY_F15 ;break;					
	case GLFW_KEY_F16:		t = KeyType::KEY_F16;break;
	case GLFW_KEY_F17:		t = KeyType::KEY_F17;break;
	case GLFW_KEY_F18:		t = KeyType::KEY_F18;break;
	case GLFW_KEY_F19:		t = KeyType::KEY_F19;break;
	case GLFW_KEY_F20:		t = KeyType::KEY_F20;break;
	case GLFW_KEY_F21:		t = KeyType::KEY_F21;break;
	case GLFW_KEY_F22:		t = KeyType::KEY_F22;break;
	case GLFW_KEY_F23:		t = KeyType::KEY_F23 ;break;					
	case GLFW_KEY_F24:		t = KeyType::KEY_F24;break;
	case GLFW_KEY_F25:		t = KeyType::KEY_F25;break;
	case GLFW_KEY_KP_0:		t = KeyType::KEY_PAD_0;break;
	case GLFW_KEY_KP_1:		t = KeyType::KEY_PAD_1;break;
	case GLFW_KEY_KP_2:		t = KeyType::KEY_PAD_2;break;
	case GLFW_KEY_KP_3:		t = KeyType::KEY_PAD_3;break;
	case GLFW_KEY_KP_4:		t = KeyType::KEY_PAD_4;break;
	case GLFW_KEY_KP_5:		t = KeyType::KEY_PAD_5 ;break;					
	case GLFW_KEY_KP_6:		t = KeyType::KEY_PAD_6;break;
	case GLFW_KEY_KP_7:		t = KeyType::KEY_PAD_7;break;
	case GLFW_KEY_KP_8:		t = KeyType::KEY_PAD_8;break;
	case GLFW_KEY_KP_9:		t = KeyType::KEY_PAD_9;break;
	case GLFW_KEY_KP_DECIMAL: t = KeyType::KEY_PAD_DECIMAL;break;
	case GLFW_KEY_KP_DIVIDE: t = KeyType::KEY_PAD_DIVIDE;break;
	case GLFW_KEY_KP_MULTIPLY: t = KeyType::KEY_PAD_MULTIPLY;break;
	case GLFW_KEY_KP_SUBTRACT: t =	KeyType::KEY_PAD_SUBTRACT ;break;					
	case GLFW_KEY_KP_ADD:		t = KeyType::KEY_PAD_ADD;break;
	case GLFW_KEY_KP_ENTER:		t = KeyType::KEY_PAD_ENTER;break;
	case GLFW_KEY_KP_EQUAL:		t = KeyType::KEY_PAD_EQUAL;break;
	case GLFW_KEY_LEFT_SHIFT:	t = KeyType::KEY_LEFT_SHIFT;break;
	case GLFW_KEY_LEFT_CONTROL:	t = KeyType::KEY_LEFT_CTRL;break;
	case GLFW_KEY_LEFT_ALT:		t = KeyType::KEY_LEFT_ALT;break;
	case GLFW_KEY_LEFT_SUPER:	t = KeyType::KEY_LEFT_SUPER;break;
	case GLFW_KEY_RIGHT_SHIFT:	t = KeyType::KEY_RIGHT_SHIFT ;break;					
	case GLFW_KEY_RIGHT_CONTROL:t = KeyType::KEY_RIGHT_CTRL;break;
	case GLFW_KEY_RIGHT_ALT:	t = KeyType::KEY_RIGHT_ALT;break;
	case GLFW_KEY_RIGHT_SUPER:	t = KeyType::KEY_RIGHT_SUPER;break;
	case GLFW_KEY_MENU:			t = KeyType::KEY_MENU; // same a key_lastbreak;
	default: t = KeyType::KEY_UNKNOWN;break;
	}
	
	client->onKeyCallBack(t, scancode, a, m);
}