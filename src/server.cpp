#include "server.h"

Server::Server(int port, QueueManager &qm) : port(port), queueManager(qm), serverSocket(INVALID_SOCKET)
{
}

Server::~Server()
{
	stop();
}

std::atomic<int> Server::clientCounter{1};

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
	std::string clientId = "client" + std::to_string(clientCounter++);

	char buffer[1024];
	while (true)
	{
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesReceived <= 0)
		{
			std::cout << "Sem conexao com " << clientId << std::endl;
			break;
		}

		std::string payload(buffer, bytesReceived);

		Message message("1");
		message.addPayload(payload);
		queueManager.push(clientId, message);

		std::cout << "Mensagem recebida do " << clientId << ": " << payload << std::endl;
		queueManager.print();
	}

	closesocket(clientSocket);
}
