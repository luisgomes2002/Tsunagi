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

		std::cout << "\n\n"
				  << std::endl;

		queueManager.print();
	}

	closesocket(clientSocket);
}
