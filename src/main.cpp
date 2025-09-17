#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "queue_manager.h"
#include "server.h"

QueueManager qm;

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Erro ao inicializar Winsock\n";
		return 1;
	};

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cerr << "Erro ao criar socket\n";
		return 1;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5000);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Erro ao bindar socket\n";
		return 1;
	}

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Erro ao colocar socket em listen\n";
		return 1;
	}

	std::cout << "Servidor TCP iniciado na porta 5000\n";

	std::thread c1(consumerThread);
	std::thread c2(consumerThread);
	std::thread c3(consumerThread);
	c1.detach();
	c2.detach();
	c3.detach();

	while (true)
	{
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		if (clientSocket != INVALID_SOCKET)
		{
			std::thread(clientHandler, clientSocket).detach();
		}
	}

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

// Criar queue_manager com uma fila simples em memória.

// Implementar append-only para persistir mensagens recebidas.

// Criar servidor TCP que escuta conexões de clientes.

// Criar protocolo simples (tipo: [tipo][tamanho][payload]) para enviar/receber mensagens.

// Criar cliente Python só para testar envio e consumo de mensagens.

// Adicionar threads para consumidores processando filas em paralelo.

// Adicionar métricas simples (quantidade de mensagens, tempo médio).