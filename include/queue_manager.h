#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>

#include "message.h"

class QueueManager
{
private:
	std::unordered_map<std::string, std::vector<Message>> queues;
	std::mutex mutex;

public:
	QueueManager();
	~QueueManager();

	void push(const std::string &clientId, const Message &message);
	bool pop(const std::string &clientId, Message &message);
	void print();
	void startConsumer(const std::string &clientId);
};

#endif