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
