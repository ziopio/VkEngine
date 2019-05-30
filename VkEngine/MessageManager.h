#pragma once
#include <vector>
#include <queue>

enum Message {
	SHUT_DOWN,
	MULTITHREADED_RENDERING_ON_OFF
};

class MsgReceiver
{
public:
	/*
	* Receives the message and takes action if triggered
	*/
	virtual void receiveMessage(Message msg) = 0;
private:
	std::queue<Message> newMessages;
};

class MessageManager
{
public:
	MessageManager();
	void registerListener(MsgReceiver* receiver);
	/*
	* The Message will be saved and later dispatched to all registered receivers.
	*/
	void sendMessage(Message msg);
	/*
	* Notifies all registered receivers with the messages stored, then clears the message queue.
	* Must be called by the main engine class.
	*/
	void dispatchMessages();
	~MessageManager();
private:
	std::vector<MsgReceiver*> receivers;
	std::queue<Message>messageQueue;
};

