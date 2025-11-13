#include "queue_manager.h"
#include <thread>
#include <iostream>

QueueManager::QueueManager() : running(true)
{
	std::cout << "Queue iniciada" << std::endl;
	monitorThread = std::thread([this]()
								{ monitorExpirations(); });
}

QueueManager::~QueueManager()
{
	stopMonitoring();
	std::cout << "Queue destruida" << std::endl;
}

void QueueManager::stopMonitoring()
{
	running = false;
	if (monitorThread.joinable())
		monitorThread.join();
}

void QueueManager::setNotifyCallback(std::function<void(const std::string &, const std::string &)> callback)
{
	notifyCallback = callback;
}

void QueueManager::publish(const std::string &clientId, const std::string &queueId, const Message &message)
{
	auto &queue = queues[clientId][queueId];
	std::lock_guard<std::mutex> lock(queue.mtx);

	queue.type = "rush"; // padrão sem tempo

	for (auto &msg : queue.messages)
	{
		if (msg.getId() == message.getId())
		{
			for (auto &payload : message.getPayloads())
				msg.addPayload(payload);
			return;
		}
	}

	queue.messages.push_back(message);
}

void QueueManager::publish(const std::string &clientId, const std::string &queueId, const Message &message, std::chrono::milliseconds lifetime)
{
	auto &queue = queues[clientId][queueId];
	std::lock_guard<std::mutex> lock(queue.mtx);

	queue.type = "rushTimed"; // fila com tempo de vida

	for (auto &msg : queue.messages)
	{
		if (msg.getId() == message.getId())
		{
			for (auto &payload : message.getPayloads())
				msg.addPayload(payload);
			msg.setLifetime(lifetime);
			return;
		}
	}

	Message msg = message;
	msg.setLifetime(lifetime);
	queue.messages.push_back(msg);
}

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

bool QueueManager::consumeRushTimed(const std::string &clientId, const std::string &queueId, Message &message)
{
	auto &queue = queues[clientId][queueId];
	std::lock_guard<std::mutex> lock(queue.mtx);

	if (!queue.messages.empty() && queue.messages.front().isExpired())
	{
		message = queue.messages.front();
		queue.messages.pop_front();

		if (queue.messages.empty())
			queues[clientId].erase(queueId);

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

		if (queue.messages.front().getPayloads().empty())
			queue.messages.pop_front();
		return true;
	}
	return false;
}

void QueueManager::monitorExpirations()
{
	while (running)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		std::lock_guard<std::mutex> lock(mutex);

		std::vector<std::pair<std::string, std::string>> queuesToRemove;

		for (auto &clientPair : queues)
		{
			for (auto &queuePair : clientPair.second)
			{
				auto &queue = queuePair.second;

				// Só verifica expiração em filas temporizadas
				if (queue.type != "rushTimed" || queue.messages.empty())
					continue;

				bool queueExpired = false;

				for (auto &msg : queue.messages)
				{
					if (msg.isExpired())
					{
						std::cout << "Mensagem expirada: "
								  << "Cliente=" << clientPair.first
								  << " | Fila=" << queuePair.first
								  << " | Tipo=" << queue.type << std::endl;

						// 🔔 Notifica o cliente apenas para rushTimed
						if (notifyCallback)
							notifyCallback(clientPair.first, queuePair.first);

						std::this_thread::sleep_for(std::chrono::milliseconds(10));

						queue.messages.pop_front();

						if (queue.messages.empty())
							queuesToRemove.emplace_back(clientPair.first, queuePair.first);

						queueExpired = true;
						break; // Evita iterador inválido
					}
				}

				if (queueExpired)
					break;
			}
		}

		for (auto &[clientId, queueName] : queuesToRemove)
		{
			auto clientIt = queues.find(clientId);
			if (clientIt != queues.end())
			{
				auto &clientQueues = clientIt->second;
				clientQueues.erase(queueName);
				if (clientQueues.empty())
					queues.erase(clientIt);
			}
		}
	}
}

std::string QueueManager::getQueueType(const std::string &clientId, const std::string &queueId)
{
	auto clientIt = queues.find(clientId);
	if (clientIt != queues.end())
	{
		auto queueIt = clientIt->second.find(queueId);
		if (queueIt != clientIt->second.end())
			return queueIt->second.type;
	}
	return "rush"; // valor padrão caso não exista
}

void QueueManager::setQueueType(const std::string &clientId, const std::string &queueId, const std::string &type)
{
	queues[clientId][queueId].type = type;
}
