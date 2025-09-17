#include "queue_manager.h"

QueueManager::QueueManager(const std::string &logFilename)
{
	std::filesystem::path logPath(logFilename);

	if (!std::filesystem::exists(logPath.parent_path()))
	{
		std::filesystem::create_directories(logPath.parent_path());
	}

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

std::string QueueManager::getTimestamp()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
	return ss.str();
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

	if (logFile.is_open())
	{
		logFile << "[" << getTimestamp() << "] " << queueName << ": " << message << "\n";
		logFile.flush();
	}

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