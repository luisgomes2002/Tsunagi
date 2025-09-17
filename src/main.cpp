#include <iostream>

#include "queue_manager.h"
#include "server.h"

int main()
{
	QueueManager queueManager;

	std::string client1 = "client1";
	std::string client2 = "client2";
	std::string client3 = "client3";

	Message msg1("123");
	msg1.addPayload("Oi, tudo bem?");
	msg1.addPayload("certo");
	msg1.addPayload("vc vai?");
	queueManager.push(client1, msg1);

	Message msg2("234");
	msg2.addPayload("Voce recebeu um pedido");
	queueManager.push(client2, msg2);

	Message msg3("542");
	msg3.addPayload("Pedido #123");
	queueManager.push(client3, msg3);

	queueManager.print();

	std::cin.get();

	return 0;
}

// Criar queue_manager com uma fila simples em memória.

// Implementar append-only para persistir mensagens recebidas.

// Criar servidor TCP que escuta conexões de clientes.

// Criar protocolo simples (tipo: [tipo][tamanho][payload]) para enviar/receber mensagens.

// Criar cliente Python só para testar envio e consumo de mensagens.

// Adicionar threads para consumidores processando filas em paralelo.

// Adicionar métricas simples (quantidade de mensagens, tempo médio).