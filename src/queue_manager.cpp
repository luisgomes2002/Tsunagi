#include "queue_manager.h"

QueueManager::QueueManager()
{
	std::cout << "Queue iniciada" << std::endl;
};

QueueManager::~QueueManager()
{
	std::cout << "Queue destruida" << std::endl;
};

void QueueManager::push(const std::string &clientId, const std::string &queueId, const Message &message)
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
		}

		return;
	}

	queue.messages.push_back(message);
};

bool QueueManager::pop(const std::string &clientId, const std::string &queueId, Message &message)
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

void QueueManager::print()
{
	for (auto &clientPair : queues)
	{
		std::cout << clientPair.first << ":\n";
		for (auto &queuePair : clientPair.second)
		{
			auto &queue = queuePair.second;
			std::lock_guard<std::mutex> lock(queue.mtx);

			for (const auto &msg : queue.messages)
			{
				std::cout << "    id: " << msg.getId() << ", payloads: [ ";
				for (const auto &p : msg.getPayloads())
					std::cout << p << " ";
				std::cout << "]\n";
			}
		}
	}
}

void QueueManager::startConsumer(const std::string &clientId, const std::string &queueId)
{
	std::thread([this, clientId, queueId]()
				{
        while (true) {
            Message message("dummy");
            if (pop(clientId, queueId, message)) {
                std::cout << "\nConsumindo message id: " 
                          << message.getId() << " do " << clientId << "\n";
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } })
		.detach();
}