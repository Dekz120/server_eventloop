#include "server.hpp"

static constexpr int listener_queue = 8;
Server::Server(size_t port) : Node()
{
    int fdscr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setFd(fdscr);
    if (fdscr < 0)
        throw serverExcept("socket()");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int bnd = bind(fdscr, (struct sockaddr *)&addr, sizeof(addr));
    if (bnd < 0)
        throw serverExcept("bind()");

    int lsn = listen(fdscr, listener_queue);
    if (lsn < 0)
        throw serverExcept("listen()");
}
Server::Server(Server &&rhs) : Node(static_cast<Node &&>(rhs)),
                               addr(std::move(rhs.addr)){};

std::shared_ptr<Node> Server::handleConnection()
{
    int accept_sd = accept(getFd(), (sockaddr *)&addr, (socklen_t *)&addr);
    if (accept_sd < 0)
        throw serverExcept("accept()");
    return std::shared_ptr<Node> (new Client(accept_sd));
}

size_t Server::getFd()
{
    return Node::getFd();
};