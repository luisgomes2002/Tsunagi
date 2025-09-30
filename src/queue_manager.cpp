#include "queue_manager.h"

QueueManager::QueueManager()
{
	std::cout << "Queue iniciada" << std::endl;
};

QueueManager::~QueueManager()
{
	std::cout << "Queue destruida" << std::endl;
};

void QueueManager::publish(const std::string &clientId, const std::string &queueId, const Message &message)
{
	auto &queue = queues[clientId][queueId];
	std::lock_guard<std::mutex> lock(queue.mtx);

	for (auto &msg : queue.messages)
	{
		if (msg.getId() == message.getId())
		{
			for (auto &payload : message.getPayloads())
			{
				msg.addPayload(payload);
			}
			return;
		}
	}

	queue.messages.push_back(message);
};

bool QueueManager::consumeRush(const std::string &clientId, const std::string &queueId, Message &message)
{
	auto &queue = queues[clientId][queueId];
	std::lock_guard<std::mutex> lock(queue.mtx);

	if (!queue.messages.empty())
	{
		message = queue.messages.front();
		queue.messages.pop_front();
		return true;
	}

	return false;
}

bool QueueManager::consumeSigle(const std::string &clientId, const std::string &queueId, Message &message)
{
	auto &queue = queues[clientId][queueId];
	std::lock_guard<std::mutex> lock(queue.mtx);

	if (!queue.messages.empty())
	{
		std::string payload = queue.messages.front().getFirstPayload();

		message = Message(queueId);
		message.addPayload(payload);
		queue.messages.front().removePayload();
		return true;
	}

	return false;
}
