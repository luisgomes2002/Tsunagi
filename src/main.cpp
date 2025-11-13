#include "server.h"
#include "queue_manager.h"

int main()
{
	QueueManager queueManager;
	Server server(5000, queueManager);

	if (!server.start())
	{
		std::cerr << "Falha ao iniciar o servidor\n";
		return 1;
	}

	return 0;
}

// Nao aparece o consumo da fila com tempo no log
// Alterar para qunado chegar a notificacao achar qual e a fila e envia as informacoes mas faz isso no client nao na lib
// pq assim o usuario nao tem controle das msg q recebe so e consumido
// Adicionar Json
// Encriptar os dados