#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

#include "message.h"

struct Queue
{
	std::deque<Message> messages;
	std::mutex mtx;
};

class QueueManager
{
private:
	std::unordered_map<std::string, std::unordered_map<std::string, Queue>> queues;

public:
	QueueManager();
	~QueueManager();

	void publish(const std::string &clientId, const std::string &queueId, const Message &message);
	bool consumeRush(const std::string &clientId, const std::string &queueId, Message &message);
	bool consumeSigle(const std::string &clientId, const std::string &queueId, Message &message);
};

#endif