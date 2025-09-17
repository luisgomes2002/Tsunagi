#include "queue_manager.h"

QueueManager::QueueManager(const std::string &logFilename)
{
	logFile.open(logFilename, std::ios::app);
	if (!logFile.is_open())
	{
		throw std::runtime_error("Não foi possível abrir arquivo de log");
	}
}

QueueManager::~QueueManager()
{
	logFile.close();
}

void QueueManager::push(const std::string &queueName, const std::string &message)
{
	std::unique_lock<std::mutex> lock(mtx);
	if (queues[queueName].size() >= 1000000)
	{
		std::cout << "Fila cheia, descartando mensagem\n";
		return;
	}

	queues[queueName].push(message);
	logFile << queueName << ":" << message << "\n";
	logFile.flush();
	cv.notify_one();
}

std::string QueueManager::pop(const std::string &queueName)
{
	std::unique_lock<std::mutex> lock(mtx);

	cv.wait(lock, [&]
			{ return !queues[queueName].empty(); });

	std::string msg = queues[queueName].front();
	queues[queueName].pop();
	return msg;
}