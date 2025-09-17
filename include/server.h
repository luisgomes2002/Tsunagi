#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>

void clientHandler(SOCKET clientSocket);
void consumerThread();

#endif
