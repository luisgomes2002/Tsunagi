#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <iostream>

#include "queue_manager.h"
#include "message.h"

class Server
{
private:
	SOCKET serverSocket;
	int port;
	QueueManager &queueManager;
	static std::atomic<int> clientCounter;

public:
	Server(int port, QueueManager &qm);
	~Server();

	bool start();
	void stop();

	static void handleClient(SOCKET clientSocket, QueueManager &queueManager);
};

#endif
