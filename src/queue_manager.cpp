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
	std::lock_guard<std::mutex> lock(mutex);

	auto &queue = queues[clientId][queueId];

	for (auto &msg : queue)
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

	queue.push_back(message);
};

bool QueueManager::pop(const std::string &clientId, const std::string &queueId, Message &message)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!queues[clientId].empty())
	{
		message = queues[clientId][queueId].back();
		queues[clientId][queueId].pop_back();
		return true;
	}

	return false;
}

void QueueManager::print()
{
	for (const auto &clientPair : queues)
	{
		std::cout << clientPair.first << ":\n";
		for (const auto &queuePair : clientPair.second)
		{
			for (const auto &msg : queuePair.second)
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
					while (true)
					{
						Message message("dummy");
						if (pop(clientId, queueId, message))
						{
							std::cout << "\nConsumindo message id: " << message.getId() << " do " << clientId << "\n";
						}else{
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}
					} })
		.detach();
}