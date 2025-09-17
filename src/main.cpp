#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>

#include "queue_manager.h"
#include "server.h"

QueueManager queueManager;
std::atomic<int> clientCounter{1};

void handleClient(SOCKET clientSocket)
{
	std::string clientId = "client" + std::to_string(clientCounter++);

	char buffer[1024];
	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	if (bytesReceived > 0)
	{
		std::string payload(buffer, bytesReceived);

		Message message("1");
		message.addPayload(payload);
		queueManager.push(clientId, message);

		std::cout << "Mensagem recebida do " << clientId << ": " << payload << std::endl;
		queueManager.print();
	}

	closesocket(clientSocket);
}

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Erro ao iniciar Winsock\n";
		return 1;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "Erro ao criar socket\n";
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(5000);

	if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind falhou\n";
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Listen falhou\n";
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "Servidor TCP iniciado na porta 5000\n";

	while (true)
	{
		SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET)
			continue;

		std::thread(handleClient, clientSocket).detach();
	}

	closesocket(serverSocket);
	WSACleanup();
	std::cin.get();

	return 0;
}

// Criar queue_manager com uma fila simples em memória. ✅

// Implementar append-only para persistir mensagens recebidas.

// Criar servidor TCP que escuta conexões de clientes.

// Criar protocolo simples (tipo: [tipo][tamanho][payload]) para enviar/receber mensagens.

// Criar cliente Python só para testar envio e consumo de mensagens.

// Adicionar threads para consumidores processando filas em paralelo. ✅

// Adicionar métricas simples (quantidade de mensagens, tempo médio).