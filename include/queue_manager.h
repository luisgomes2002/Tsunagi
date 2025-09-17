#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <string>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <iomanip>

#include <filesystem>

class QueueManager
{
private:
	std::unordered_map<std::string, std::queue<std::string>> queues;
	std::mutex mtx;
	std::condition_variable cv;
	std::ofstream logFile;

	std::string getTimestamp();

public:
	QueueManager(const std::string &logFilename = "../logs/messages.txt");
	~QueueManager();

	void push(const std::string &queueName, const std::string &message);

	std::string pop(const std::string &queueName);
};

#endif