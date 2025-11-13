#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>
#include <chrono>

class Message
{
private:
	std::string id;
	std::vector<std::string> payloads;
	std::chrono::steady_clock::time_point creationTime;
	std::chrono::milliseconds lifetime;

public:
	Message(const std::string &id);
	~Message();

	std::string getId() const;
	const std::vector<std::string> &getPayloads() const;

	void addPayload(const std::string &data);
	void removePayload();
	std::string getFirstPayload();

	void setLifetime(std::chrono::milliseconds duration);
	bool isExpired() const;
};

#endif
