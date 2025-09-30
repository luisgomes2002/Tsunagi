#include "server.h"

Server::Server(int port, QueueManager &queueManager) : port(port), queueManager(queueManager), serverSocket(INVALID_SOCKET)
{
}

Server::~Server()
{
	stop();
}

bool Server::recvAll(SOCKET socket, char *buffer, int totalBytes)
{
	int recived = 0;
	while (recived < totalBytes)
	{
		int r = recv(socket, buffer + recived, totalBytes - recived, 0);
		if (r <= 0)
			return 0;
		recived += r;
	}

	return true;
}

bool Server::start()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Erro ao iniciar Winsock\n";
		return false;
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "Erro ao criar socket\n";
		WSACleanup();
		return false;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind falhou\n";
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Listen falhou\n";
		closesocket(serverSocket);
		WSACleanup();
		return false;
	}

	std::cout << "Servidor TCP iniciado na porta " << port << "\n";

	while (true)
	{
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET)
			continue;

		std::thread(handleClient, clientSocket, std::ref(queueManager)).detach();
	}

	return true;
}

void Server::stop()
{
	if (serverSocket != INVALID_SOCKET)
	{
		closesocket(serverSocket);
		serverSocket = INVALID_SOCKET;
	}
	WSACleanup();
}

void Server::rush(std::string &rest, std::string &clientName, SOCKET &clientSocket, QueueManager &queueManager)
{
	std::string queueId = rest;
	Message message("");
	std::string allMessages;

	while (queueManager.consumeRush(clientName, queueId, message))
	{
		for (const auto &payload : message.getPayloads())
		{
			if (!allMessages.empty())
				allMessages += "\n";
			allMessages += payload;
		}
	}

	if (allMessages.empty())
	{
		allMessages = "EMPTY";
	}

	int size = (int)allMessages.size();
	send(clientSocket, reinterpret_cast<char *>(&size), sizeof(size), 0);
	send(clientSocket, allMessages.c_str(), size, 0);

	std::cout << "Cliente " << clientName
			  << " consumiu todas msgs da fila " << queueId << "\n";
}

void Server::single(std::string &rest, std::string &clientName, SOCKET &clientSocket, QueueManager &queueManager)
{
	std::string queueId = rest;
	Message message("");
	std::string msg;

	if (queueManager.consumeSigle(clientName, queueId, message))
	{
		msg = message.getPayloads().front();
		int size = (int)msg.size();
		send(clientSocket, reinterpret_cast<char *>(&size), sizeof(size), 0);
		send(clientSocket, msg.c_str(), size, 0);

		std::cout << "Cliente " << clientName
				  << " consumiu UMA mensagem da fila " << queueId << "\n";
	}
	else
	{
		msg = "EMPTY";
		int size = (int)msg.size();
		send(clientSocket, reinterpret_cast<char *>(&size), sizeof(size), 0);
		send(clientSocket, msg.c_str(), size, 0);
	}

	std::cout << "Cliente " << clientName
			  << " tentou consumir da fila " << queueId << " mas estava vazia\n";
}

void Server::handleClient(SOCKET clientSocket, QueueManager &queueManager)
{

	int nameSize = 0;
	if (!recvAll(clientSocket, reinterpret_cast<char *>(&nameSize), sizeof(nameSize)))
	{
		std::cout << "Cliente desconectou antes de enviar o indentificador.\n\n";
		closesocket(clientSocket);
		return;
	}

	std::vector<char> nameBuffer(nameSize);
	if (!recvAll(clientSocket, nameBuffer.data(), nameSize))
	{
		std::cerr << "Falha ao receber nome do cliente\n";
		closesocket(clientSocket);
		return;
	}

	std::string clientName(nameBuffer.begin(), nameBuffer.end());
	std::cout << "Cliente conectado: " << clientName << "\n\n";

	while (true)
	{
		int msgSize = 0;
		int r = recv(clientSocket, reinterpret_cast<char *>(&msgSize), sizeof(msgSize), 0);
		if (r <= 0)
		{
			std::cout << "Cliente " << clientName << " desconectou.\n";
			break;
		}

		std::vector<char> msgBuffer(msgSize);
		if (!recvAll(clientSocket, msgBuffer.data(), msgSize))
		{
			std::cerr << "Falha ao receber mensagem completa\n";
			break;
		}

		std::string received(msgBuffer.begin(), msgBuffer.end());

		size_t sep = received.find('|');
		std::string command;
		std::string rest;

		if (sep != std::string::npos)
		{
			command = received.substr(0, sep);
			rest = received.substr(sep + 1);
		}
		else
		{
			command = "PUB";
			rest = received;
		}

		if (command == "PUB")
		{
			size_t sep2 = rest.find('|');
			std::string queueId;
			std::string payload;

			if (sep2 != std::string::npos)
			{
				queueId = rest.substr(0, sep2);
				payload = rest.substr(sep2 + 1);
			}
			else
			{
				queueId = "default";
				payload = rest;
			}

			Message message(queueId);
			message.addPayload(payload);
			queueManager.publish(clientName, queueId, message);

			std::cout << "Mensagem publicada na fila " << queueId << "\n";
		}
		else if (command == "CON")
		{
			single(rest, clientName, clientSocket, queueManager);
		}
	}

	closesocket(clientSocket);
}
