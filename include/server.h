#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#include "queue_manager.h"
#include "message.h"

class Server
{
private:
	SOCKET serverSocket;
	int port;
	QueueManager &queueManager;

	static bool recvAll(SOCKET socket, char *buffer, int totalBytes);

public:
	Server(int port, QueueManager &queueManager);
	~Server();

	bool start();
	void stop();

	static void handleClient(SOCKET clientSocket, QueueManager &queueManager);

	// Consumers
	static void rush(std::string &rest, std::string &clientName, SOCKET &clientSocket, QueueManager &queueManager);
	static void single(std::string &rest, std::string &clientName, SOCKET &clientSocket, QueueManager &queueManager);
};

#endif
