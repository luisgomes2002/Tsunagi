#include "server.h"

Server::Server(int port, QueueManager &queueManager) : port(port), queueManager(queueManager), serverSocket(INVALID_SOCKET)
{
}

Server::~Server()
{
	stop();
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

void Server::handleClient(SOCKET clientSocket, QueueManager &queueManager)
{

	char buffer[1024];
	while (true)
	{
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesReceived <= 0)
		{
			std::cout << "Cliente desconectou antes de enviar o indentificador.\n\n";
			closesocket(clientSocket);
			return;
		}

		std::string clientName(buffer, bytesReceived);
		std::cout << "Cliente conectado: " << clientName << "\n\n";

		while (true)
		{
			bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
			if (bytesReceived <= 0)
			{
				std::cout << "\nCliente " << clientName << " desconectou.\n";
				break;
			}

			std::string received(buffer, bytesReceived);

			size_t sep = received.find('|');
			std::string messageId;
			std::string payload;

			if (sep != std::string::npos)
			{
				messageId = received.substr(0, sep);
				payload = received.substr(sep + 1);
			}
			else
			{
				messageId = "unknown";
				payload = received;
			}

			Message message(messageId);
			message.addPayload(payload);
			queueManager.push(clientName, messageId, message);

			std::cout << "------------------------------------------------------------------" << std::endl;
			queueManager.print();
		}
	}

	closesocket(clientSocket);
}
