#include "client.hpp"

Client::Client(size_t descr) : Node(descr) {}
Client::Client(Client &&rhs) : Node(static_cast<Node &&>(rhs)),
                               request_field(std::move(rhs.request_field)) {}

int Client::handleConnection()
{
    char buff[512];
    size_t rcv;

    rcv = recv(getFd(), buff, 512, 0);

    if (rcv < 0)
    {
        return -1;
    }
    if (rcv == 0)
    {
        Node::closeConnection();
        return 0;
    }
    if (rcv > 0)
    {
        request_field.append(buff, rcv);
        auto p = request_field.find('\n');
        if (p != std::string::npos)
        {

            response = request_field.substr(0, p + 1);
            sendData();
            request_field.clear();
        }
    }
    return 0;
}

void Client::prepateData()
{
    if (response.substr(0, 5) == "time\n")
        Client::prepareTime();
    else if (response.substr(0, 5) == "echo ")
        Client::prepareEcho();
    else
        response = "Wrong request\n";
}

void Client::prepareEcho()
{
    response = response.substr(5, request_field.size() - 5);
}

void Client::prepareTime()
{
    auto t = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(t);
    response = std::ctime(&tt);
}

void Client::sendData()
{
    prepateData();
    int s = send(getFd(), response.c_str(), response.size(), 0);
    if (s < 0)
        throw serverExcept("send()");
}
