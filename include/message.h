#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <cstdint>
#include <winsock2.h>

struct Message
{
	uint8_t type;
	uint32_t size;
	std::string payload;
};

inline Message readMessage(SOCKET s)
{
	Message msg;

	int ret = recv(s, reinterpret_cast<char *>(&msg.type), 1, 0);
	if (ret <= 0)
		throw std::runtime_error("Cliente desconectado");

	uint32_t sizeNet;
	recv(s, reinterpret_cast<char *>(&sizeNet), 4, 0);
	if (ret <= 0)
		throw std::runtime_error("Cliente desconectado");
	msg.size = ntohl(sizeNet);

	char *buffer = new char[msg.size];
	ret = recv(s, buffer, msg.size, 0);
	if (ret <= 0)
	{
		delete[] buffer;
		throw std::runtime_error("Cliente desconectado");
	}

	msg.payload.assign(buffer, msg.size);
	delete[] buffer;

	return msg;
}

inline void sendMessage(SOCKET s, const Message &msg)
{
	uint32_t sizeNet = htonl(msg.size);

	send(s, reinterpret_cast<const char *>(&msg.type), 1, 0);
	send(s, reinterpret_cast<const char *>(&sizeNet), 4, 0);
	send(s, msg.payload.c_str(), msg.size, 0);
}

#endif