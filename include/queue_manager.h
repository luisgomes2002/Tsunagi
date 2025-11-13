#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>
#include <chrono>
#include <atomic>
#include <functional>

#include "message.h"

struct Queue
{
	std::deque<Message> messages;
	std::mutex mtx;
	std::string type;
};

class QueueManager
{
private:
	std::thread monitorThread;
	std::atomic<bool> running;
	std::unordered_map<std::string, std::unordered_map<std::string, Queue>> queues;

	std::function<void(const std::string &, const std::string &)> notifyCallback;

	void monitorExpirations();

	std::mutex mutex;

public:
	QueueManager();
	~QueueManager();

	void setNotifyCallback(std::function<void(const std::string &, const std::string &)> callback);

	void publish(const std::string &clientId, const std::string &queueId, const Message &message);
	void publish(const std::string &clientId, const std::string &queueId, const Message &message, std::chrono::milliseconds duration);

	bool consumeRush(const std::string &clientId, const std::string &queueId, Message &message);
	bool consumeRushTimed(const std::string &clientId, const std::string &queueId, Message &message);
	bool consumeSigle(const std::string &clientId, const std::string &queueId, Message &message);

	void stopMonitoring();

	std::string getQueueType(const std::string &clientId, const std::string &queueId);
	void setQueueType(const std::string &clientId, const std::string &queueId, const std::string &type);
};

#endif
