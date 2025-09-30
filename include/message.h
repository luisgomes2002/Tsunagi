#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>

class Message
{
private:
	std::string id;
	std::vector<std::string> payloads;

public:
	Message(const std::string &id);
	~Message();

	std::string getId() const;
	const std::vector<std::string> &getPayloads() const;

	void addPayload(const std::string &data);
	void removePayload();

	std::string getFirstPayload();
};

#endif