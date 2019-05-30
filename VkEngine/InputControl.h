#pragma once
#include "VkEngine.h"

enum InputEventType {
	MOUSE_L_CLICK,
	MOUSE_R_CLICK,
	SPACE, ESCAPE,
	MOUSE_POSITION,
	MOVE_FOREWARD,
	MOVE_RIGHT,
	MOVE_LEFT,
	MOVE_BACK,
	MOVE_DOWN,
	STOP_MOVE_FOREWARD,
	STOP_MOVE_RIGHT,
	STOP_MOVE_LEFT,
	STOP_MOVE_UP,
	STOP_MOVE_DOWN,
	STOP_MOVE_BACK,
	PICK_OBJECT,
	ENTER,
	SHIFT_DOWN,
	SHIFT_UP
};

class InputEvent {
public:
	InputEvent(InputEventType type);
	InputEvent(InputEventType type, int x, int y);
	InputEventType getType();
	int getX();
	int getY();
private:
	InputEventType type;
	int x;
	int y;
};
class InputControl
{
public:
	static void init(MessageManager* msgManager);
	static void processInput();
private:
	static MessageManager* msgManager;
	static std::queue<InputEvent> events;
	static bool ready;
};

