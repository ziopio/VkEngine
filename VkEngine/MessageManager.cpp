#include "stdafx.h"
#include "MessageManager.h"


MessageManager::MessageManager()
{
}

void MessageManager::registerListener(MsgReceiver* receiver)
{
	this->receivers.push_back(receiver);
}

void MessageManager::sendMessage(Message msg)
{
	this->messageQueue.push(msg);
}

void MessageManager::dispatchMessages()
{
	while (messageQueue.size() > 0) {
		Message msg = messageQueue.front();
		messageQueue.pop();
		for (auto receiver : this->receivers) {
			receiver->receiveMessage(msg);
		}
	}
}

MessageManager::~MessageManager()
{
}
