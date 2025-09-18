#include "queue_manager.h"

QueueManager::QueueManager()
{
	std::cout << "Queue iniciada" << std::endl;
};

QueueManager::~QueueManager()
{
	std::cout << "Queue destruida" << std::endl;
};

void QueueManager::push(const std::string &clientId, const Message &message)
{
	std::lock_guard<std::mutex> lock(mutex);

	auto &queue = queues[clientId];

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

	queues[clientId].push_back(message);
};

bool QueueManager::pop(const std::string &clientId, Message &message)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (!queues[clientId].empty())
	{
		message = queues[clientId].back();
		queues[clientId].pop_back();
		return true;
	}

	return false;
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

void QueueManager::startConsumer(const std::string &clientId)
{
	std::thread([this, clientId]()
				{
					while (true)
					{
						Message message("dummy");
						if (pop(clientId, message))
						{
							std::cout << "\nConsumindo message id: " << message.getId() << " do " << clientId << "\n";
						}else{
							std::this_thread::sleep_for(std::chrono::milliseconds(100));
						}
					} })
		.detach();
}