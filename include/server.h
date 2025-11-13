#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <mutex>

#include "queue_manager.h"
#include "message.h"

class Server
{
private:
	int port;
	SOCKET serverSocket;
	QueueManager &queueManager;

	static bool recvAll(SOCKET socket, char *buffer, int totalBytes);

	std::unordered_map<std::string, SOCKET> clientSockets;
	std::mutex clientMtx;

public:
	Server(int port, QueueManager &queueManager);
	~Server();

	bool start();
	void stop();

	void notifyClient(const std::string &clientId, const std::string &queueId);

	static void handleClient(SOCKET clientSocket, QueueManager &queueManager, Server *instance);

	static void rush(std::string &rest, std::string &clientName, SOCKET &clientSocket, QueueManager &queueManager);
	static void single(std::string &rest, std::string &clientName, SOCKET &clientSocket, QueueManager &queueManager);
};

#endif
