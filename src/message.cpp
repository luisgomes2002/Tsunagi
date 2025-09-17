#include "Message.h"

Message::Message(const std::string &id) : id(id) {};

Message::~Message() {};

std::string Message::getId() const { return id; };
const std::vector<std::string> &Message::getPayloads() const { return payloads; };

void Message::addPayload(const std::string &data) { payloads.push_back(data); }