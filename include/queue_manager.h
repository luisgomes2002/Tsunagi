#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "message.h"

class QueueManager
{
private:
	std::unordered_map<std::string, std::vector<Message>> queues;

public:
	QueueManager();
	~QueueManager();

	void push(std::string &clientId, Message &Message);
	void pop(std::string &clientId);

	void print();
};

#endif