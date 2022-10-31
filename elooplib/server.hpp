#pragma once

#include "client.hpp"

class Server : public Node
{
private:
    sockaddr_in addr;

public:
    Server(size_t);
    Server(Server &&);
    std::shared_ptr<Node> handleConnection() override;
    size_t getFd() override;
};
