#include "node.hpp"

Node::Node(size_t descr) : fd(descr), active(1){};
Node::Node() : fd(0), active(1){};
Node::Node(Node &&rhs) : fd(rhs.fd), active(rhs.active)
{
    rhs.fd = -1;
    rhs.active = 0;
}

Node::~Node()
{
    if (fd > 0)
        close(fd);
}

std::shared_ptr<Node> Node::handleConnection() { return nullptr; };
void Node::closeConnection()
{
    active = 0;
}
size_t Node::getFd()
{
    return fd;
}

void Node::setFd(int d)
{
    fd = d;
}

bool Node::is_active()
{
    return active;
}
