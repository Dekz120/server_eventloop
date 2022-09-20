#ifndef SERVER_INCLUDED
#define SERVER_INCLUDED

#include "node.hpp"

class Server : public Node
{
    private:
        sockaddr_in addr;
    public:
        Server(size_t);
        Server(Server&&);
        int handleConnection() override;
};

#endif //SERVER_INCLUDED
