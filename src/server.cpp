#include <iostream>
#include <thread>

#include "server.h"
#include "queue_manager.h"
#include "message.h"

extern QueueManager qm;

void consumerThread()
{
	while (true)
	{
		std::string msg = qm.pop("fila");
		std::cout << "[Consumidor] Processando: " << msg << "\n";

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void clientHandler(SOCKET clientSocket)
{
	while (true)
	{
		Message msg;
		try
		{
			msg = readMessage(clientSocket);
		}
		catch (...)
		{
			break;
		}
		qm.push("fila", msg.payload);
	}
	closesocket(clientSocket);
}
