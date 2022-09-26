#include "eventloop.hpp"

EventLoop::EventLoop(size_t n_clients) : fds(n_clients), maxPollfdPos(0){};

void EventLoop::addNode(std::shared_ptr<Node> &node, bool nonblock)
{
    int p_pos;

    if (availablePos.empty())
    {
        p_pos = maxPollfdPos++;
    }
    else
    {
        p_pos = availablePos.front();
        availablePos.pop();
    }

    int fd = node->getFd();

    if (nonblock)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        flags = (flags | O_NONBLOCK);
        fcntl(fd, F_SETFL, flags);
    }

    fds[p_pos].fd = fd;
    fds[p_pos].events = POLLIN;

    nodes.push_back(std::make_pair(std::move(p_pos), node));
}

EventLoop::nodeRetType EventLoop::rmNode(nodeRetType &nodep)
{
    if (nodep->first == maxPollfdPos - 1 && availablePos.empty())
        maxPollfdPos--;
    else
        availablePos.push(nodep->first);

    fds[nodep->first].fd = -1;
    nodep->first = -1;

    auto ret = nodes.erase(nodep);
    return --ret;
}

int EventLoop::handleConnection(std::shared_ptr<Node> &hst)
{
    auto client_ptr = std::dynamic_pointer_cast<Client>(hst);
    if (client_ptr)
    {
        std::cout << "Client handler\n";
        return client_ptr->handleConnection();
    }
    else
    {
        auto serv_ptr = std::dynamic_pointer_cast<Server>(hst);
        if (serv_ptr)
        {
            std::cout << "Server handler\n";
            return serv_ptr->handleConnection();
        }
    }
    return -1;
}

void EventLoop::run()
{
    while (!stop)
    {
        int rc = poll(fds.data(), maxPollfdPos, 0);
        if (rc < 0)
            throw serverExcept("poll()");

        for (nodeRetType node_p = nodes.begin(); node_p != nodes.end(); node_p++)
        {
            auto &[pos, node] = *node_p;
            if (fds[pos].revents == POLLIN)
            {
                int data = handleConnection(node);

                if (data > 0)
                {
                    auto clnt = std::shared_ptr<Node>(new Client(data));
                    addNode(clnt, 0); // TODO add nonblock support
                }
                if (!node->is_active())
                {
                    node_p = rmNode(node_p);
                }
            }
        }
        std::signal(SIGINT, EventLoop::handleSignal);
    }
}

void EventLoop::handleSignal(int signal)
{
    stop = signal;
}
