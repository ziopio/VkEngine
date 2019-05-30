#include "stdafx.h"
#include "InputControl.h"
#include "Direction.h"


MessageManager* InputControl::msgManager;
std::queue<InputEvent> InputControl::events;
bool InputControl::ready;

void InputControl::init(MessageManager* msgManager)
{
	InputControl::msgManager = msgManager;
}

void InputControl::processInput()
{
	while (events.size() > 0) {
		InputEvent input = events.front();
		events.pop();

		switch (input.getType())
		{
		case InputEventType::MOVE_FOREWARD:Direction::getCurrentCamera()->moveCameraForeward();
			break;
		case InputEventType::MOVE_LEFT:Direction::getCurrentCamera()->moveCameraLeft();
			break;
		case InputEventType::MOVE_RIGHT: Direction::getCurrentCamera()->moveCameraRight();
			break;
		case InputEventType::MOVE_BACK:Direction::getCurrentCamera()->moveCameraBack();
			break;
		case InputEventType::STOP_MOVE_FOREWARD: Direction::getCurrentCamera()->stopCameraForeward();
			break;
		case InputEventType::STOP_MOVE_LEFT:Direction::getCurrentCamera()->stopCameraLeft();
			break;
		case InputEventType::STOP_MOVE_RIGHT:Direction::getCurrentCamera()->stopCameraRight();
			break;
		case InputEventType::STOP_MOVE_BACK:Direction::getCurrentCamera()->stopmoveCameraBack();
			break;
		case InputEventType::MOUSE_POSITION:Direction::getCurrentCamera()->mouseRotation(input.getX(), input.getY());
			break;
		case InputEventType::SHIFT_DOWN:Direction::getCurrentCamera()->fastSpeedCamera();
			break;
		case InputEventType::SHIFT_UP:Direction::getCurrentCamera()->normalSpeedCamera();
			break;
		case InputEventType::ENTER: Direction::getCurrentCamera()->reset();
			break;
		case InputEventType::ESCAPE:InputControl::msgManager->sendMessage(Message::SHUT_DOWN);
			break;
		case InputEventType::SPACE:InputControl::msgManager->sendMessage(Message::MULTITHREADED_RENDERING_ON_OFF);
			break;
		default:
			break;
		}
	}
}


InputEvent::InputEvent(InputEventType type)
{
	this->type = type;
}

InputEvent::InputEvent(InputEventType type, int x, int y)
{
	this->type = type;
	this->x = x;
	this->y = y;
}

InputEventType InputEvent::getType()
{
	return this->type;
}

int InputEvent::getX()
{
	return this->x;
}

int InputEvent::getY()
{
	return this->y;
}

//
//
//
//void cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
//{
//	InputControl::events.push(InputEvent(InputEventType::MOUSE_POSITION,xpos,ypos));
//}
//
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
//{
//	if (mods & GLFW_MOD_SHIFT) {
//		InputControl::events.push(InputEvent(InputEventType::SHIFT_DOWN));
//	}
//	else {
//		InputControl::events.push(InputEvent(InputEventType::SHIFT_UP));
//	}
//	switch (action) {
//	case GLFW_PRESS:key_pressed(key);
//		break;
//	case GLFW_RELEASE:key_released(key);
//		break;
//	default:
//		break;
//	}
//}
//
//
//void key_pressed(int key) {
//	switch (key)
//	{
//	case GLFW_KEY_W:InputControl::events.push(InputEvent(InputEventType::MOVE_FOREWARD));
//		break;
//	case GLFW_KEY_A:InputControl::events.push(InputEvent(InputEventType::MOVE_LEFT));
//		break;
//	case GLFW_KEY_S:InputControl::events.push(InputEvent(InputEventType::MOVE_BACK));
//		break;
//	case GLFW_KEY_D:InputControl::events.push(InputEvent(InputEventType::MOVE_RIGHT));
//		break;
//	case GLFW_KEY_ESCAPE:InputControl::events.push(InputEvent(InputEventType::ESCAPE));
//		break;
//	case GLFW_KEY_ENTER:InputControl::events.push(InputEvent(InputEventType::ENTER));
//		break;
//	case GLFW_KEY_SPACE:InputControl::events.push(InputEvent(InputEventType::SPACE));
//		break;
//	default:
//		break;
//	}
//}
//
//void key_released(int key) {
//
//	switch (key)
//	{
//	case GLFW_KEY_W:InputControl::events.push(InputEvent(InputEventType::STOP_MOVE_FOREWARD));
//		break;
//	case GLFW_KEY_A:InputControl::events.push(InputEvent(InputEventType::STOP_MOVE_LEFT));
//		break;
//	case GLFW_KEY_S:InputControl::events.push(InputEvent(InputEventType::STOP_MOVE_BACK));
//		break;
//	case GLFW_KEY_D:InputControl::events.push(InputEvent(InputEventType::STOP_MOVE_RIGHT));
//		break;
//	default:
//		break;
//	}
//}