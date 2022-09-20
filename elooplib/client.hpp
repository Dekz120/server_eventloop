#ifndef CLIENT_INCLUDED
#define CLIENT_INCLUDED

#include "node.hpp"

class Client : public Node
{
    private:
        std::string request_field;
        std::string response;
    public:
        Client(size_t);
        Client(Client&&);
        int handleConnection() override;
        void  prepareTime();
        void  prepareEcho();
        void prepateData();
        void sendData();
};

#endif //CLIENT_INCLUDED
