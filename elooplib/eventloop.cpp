#include "eventloop.hpp"

EventLoop::EventLoop(size_t n_clients) : fds(n_clients), max_pollfd_pos(0){};

void EventLoop::addNode(Host&& node, bool nonblock)
{
    int p_pos;

    if(available_pos.empty())
    {
        p_pos = max_pollfd_pos++;
    }
    else
    {
        p_pos = available_pos.front();
        available_pos.pop();
    }

    int fd;
    if (const auto nd (std::get_if<Server>(&node)); nd)
        fd = nd->getFd();
    else if (const auto nd (std::get_if<Client>(&node)); nd)
        fd = nd->getFd();

    if(nonblock)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        flags = (flags | O_NONBLOCK);
        fcntl(fd, F_SETFL, flags);
    }

    fds[p_pos].fd = fd;
    fds[p_pos].events = POLLIN;

    nodes.push_back(std::make_pair(std::move(p_pos), std::move(node)));
}

EventLoop::nodeRetType EventLoop::rmNode(nodeRetType& nodep)
{
    if(nodep->first == max_pollfd_pos-1 && available_pos.empty())
            max_pollfd_pos--;
    else
            available_pos.push(nodep->first);

    fds[nodep->first].fd = -1;
    nodep->first = -1;

    auto ret = nodes.erase(nodep);
    return --ret;
}

int EventLoop::handleConnection(Host& hst)
{
    auto servNode = std::get_if<Server>(&hst);
    auto clientNode = std::get_if<Client>(&hst);
    if(clientNode)
        return clientNode->handleConnection();
    else
        return servNode->handleConnection();
}

bool EventLoop::isActive(Host& hst)
{
    auto servNode = std::get_if<Server>(&hst);
    auto clientNode = std::get_if<Client>(&hst);
    if(clientNode)
        return clientNode->is_active();
    else
        return servNode->is_active();
}

void EventLoop::run()
{
    while(!stop) // add signal
    {
        int rc = poll(fds.data(), max_pollfd_pos, 0);
        if(rc < 0)
            {}

        for(auto node_p = nodes.begin(); node_p != nodes.end(); node_p++)
        {

            auto &[pos, node] = *node_p;
            if(fds[pos].revents == POLLIN)
            {

                int data = handleConnection(node_p->second);
                if(data > 0)
                {
                    addNode(std::move(Client(data)), 1);
                }
                if(!isActive(node_p->second))
                {
                    node_p = rmNode(node_p);
                }
            }
        }
        std::signal(SIGINT, signalHandler::handleSignal);
    }
}
