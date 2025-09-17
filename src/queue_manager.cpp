#include "queue_manager.h"

QueueManager::QueueManager() {};

QueueManager::~QueueManager() {};

void QueueManager::push(std::string &clientId, Message &message)
{
	queues[clientId].push_back(message);
};

void QueueManager::pop(std::string &clientId)
{
}

void QueueManager::print()
{
	for (const auto &pair : queues)
	{
		std::cout << pair.first << ":\n";

		for (const auto &msg : pair.second)
		{
			std::cout << "  id: " << msg.getId() << ", payloads: [ ";

			const auto &payloads = msg.getPayloads();
			for (size_t i = 0; i < payloads.size(); i++)
			{
				std::cout << payloads[i];
				if (i < payloads.size() - 1)
					std::cout << ", ";
			}
			std::cout << " ]\n";
		}
	}
}
