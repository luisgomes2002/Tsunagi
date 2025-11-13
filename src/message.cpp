#include "message.h"

Message::Message(const std::string &id)
	: id(id), creationTime(std::chrono::steady_clock::now()), lifetime(std::chrono::milliseconds(0))
{
}

Message::~Message() {}

std::string Message::getId() const
{
	return id;
}

const std::vector<std::string> &Message::getPayloads() const
{
	return payloads;
}

void Message::addPayload(const std::string &data)
{
	payloads.push_back(data);
}

void Message::removePayload()
{
	if (!payloads.empty())
		payloads.erase(payloads.begin());
}

std::string Message::getFirstPayload()
{
	if (payloads.empty())
		return "";
	return payloads.front();
}

void Message::setLifetime(std::chrono::milliseconds duration)
{
	lifetime = duration;
	creationTime = std::chrono::steady_clock::now();
}

bool Message::isExpired() const
{
	if (lifetime.count() == 0)
		return true;
	auto now = std::chrono::steady_clock::now();
	return (now - creationTime) >= lifetime;
}
