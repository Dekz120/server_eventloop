#pragma once

#include "node.hpp"

class Server : public Node
{
private:
    sockaddr_in addr;

public:
    Server(size_t);
    Server(Server &&);
    int handleConnection() override;
};
