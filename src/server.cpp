#include "server.h"
#include "message.h"

Server::Server(int port, QueueManager &queueManager)
	: port(port), queueManager(queueManager), serverSocket(INVALID_SOCKET)
{
	// registra callback de expiração
	queueManager.setNotifyCallback([this](const std::string &client, const std::string &queueId)
								   { this->notifyClient(client, queueId); });
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
			return false;
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

		std::thread([this, clientSocket]()
					{ handleClient(clientSocket, queueManager, this); })
			.detach();
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

void Server::notifyClient(const std::string &clientId, const std::string &queueId)
{
	std::lock_guard<std::mutex> lock(clientMtx);
	auto it = clientSockets.find(clientId);
	if (it != clientSockets.end())
	{
		SOCKET sock = it->second;

		std::string type = queueManager.getQueueType(clientId, queueId);
		if (type.empty())
			type = "rush"; // fallback

		std::string msg = "EXPIRED|" + queueId + "|" + type;
		int size = (int)msg.size();
		send(sock, reinterpret_cast<char *>(&size), sizeof(size), 0);
		send(sock, msg.c_str(), size, 0);

		std::cout << "Notificacao enviada a " << clientId << ": " << msg << std::endl;
	}
}

void Server::handleClient(SOCKET clientSocket, QueueManager &queueManager, Server *instance)
{
	int nameSize = 0;
	if (!recvAll(clientSocket, reinterpret_cast<char *>(&nameSize), sizeof(nameSize)))
	{
		closesocket(clientSocket);
		return;
	}

	std::vector<char> nameBuffer(nameSize);
	if (!recvAll(clientSocket, nameBuffer.data(), nameSize))
	{
		closesocket(clientSocket);
		return;
	}

	std::string clientName(nameBuffer.begin(), nameBuffer.end());
	{
		std::lock_guard<std::mutex> lock(instance->clientMtx);
		instance->clientSockets[clientName] = clientSocket;
	}

	std::cout << "Cliente conectado: " << clientName << "\n";

	while (true)
	{
		int msgSize = 0;
		int r = recv(clientSocket, reinterpret_cast<char *>(&msgSize), sizeof(msgSize), 0);
		if (r <= 0)
			break;

		std::vector<char> msgBuffer(msgSize);
		if (!recvAll(clientSocket, msgBuffer.data(), msgSize))
			break;

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

		// ---------------------------------------------------------------------
		// PUBLICAR
		// ---------------------------------------------------------------------
		if (command == "PUB")
		{
			size_t sep2 = rest.find('|');
			std::string queueId, payload, durationStr, queueType;

			if (sep2 != std::string::npos)
			{
				queueId = rest.substr(0, sep2);
				std::string remaining = rest.substr(sep2 + 1);

				size_t sep3 = remaining.find('|');
				if (sep3 != std::string::npos)
				{
					payload = remaining.substr(0, sep3);
					std::string remaining2 = remaining.substr(sep3 + 1);

					size_t sep4 = remaining2.find('|');
					if (sep4 != std::string::npos)
					{
						durationStr = remaining2.substr(0, sep4);
						queueType = remaining2.substr(sep4 + 1);
					}
					else
					{
						durationStr = remaining2;
						queueType = "rush";
					}
				}
				else
				{
					payload = remaining;
					queueType = "rush";
				}
			}
			else
			{
				queueId = "default";
				payload = rest;
				queueType = "rush";
			}

			Message message(queueId);
			message.addPayload(payload);

			if (!durationStr.empty() && durationStr != "0")
			{
				int durationMs = std::stoi(durationStr);
				queueManager.publish(clientName, queueId, message, std::chrono::milliseconds(durationMs));
			}
			else
			{
				queueManager.publish(clientName, queueId, message);
			}

			queueManager.setQueueType(clientName, queueId, queueType);
		}

		// ---------------------------------------------------------------------
		// CONSUMO RUSH
		// ---------------------------------------------------------------------
		else if (command == "RUSH")
		{
			Message message(rest);
			if (queueManager.consumeRush(clientName, rest, message))
			{
				std::string payload = message.getFirstPayload();
				std::string response = "RUSH|" + rest + "|" + payload;

				int size = static_cast<int>(response.size());
				send(clientSocket, reinterpret_cast<char *>(&size), sizeof(size), 0);
				send(clientSocket, response.c_str(), size, 0);

				std::cout << "[RUSH] Consumido: " << payload
						  << " (cliente=" << clientName << ", fila=" << rest << ")\n";
			}
			else
			{
				std::cout << "[RUSH] Nenhuma mensagem disponível na fila "
						  << rest << " (cliente=" << clientName << ")\n";
			}
		}

		// ---------------------------------------------------------------------
		// CONSUMO SINGLE
		// ---------------------------------------------------------------------
		else if (command == "SINGLE")
		{
			Message message(rest);
			if (queueManager.consumeSigle(clientName, rest, message))
			{
				std::string payload = message.getFirstPayload();
				std::string response = "SINGLE|" + rest + "|" + payload;

				int size = static_cast<int>(response.size());
				send(clientSocket, reinterpret_cast<char *>(&size), sizeof(size), 0);
				send(clientSocket, response.c_str(), size, 0);

				std::cout << "[SINGLE] Consumido: " << payload
						  << " (cliente=" << clientName << ", fila=" << rest << ")\n";
			}
			else
			{
				std::cout << "[SINGLE] Nenhuma mensagem disponível na fila "
						  << rest << " (cliente=" << clientName << ")\n";
			}
		}
	}

	closesocket(clientSocket);
	{
		std::lock_guard<std::mutex> lock(instance->clientMtx);
		instance->clientSockets.erase(clientName);
	}
	std::cout << "Cliente desconectado: " << clientName << std::endl;
}
