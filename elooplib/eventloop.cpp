#include "eventloop.hpp"

EventLoop::EventLoop(size_t n_clients) : fds(n_clients), max_pollfd_pos(0){};

void EventLoop::addNode(std::shared_ptr<Node> &node, bool nonblock)
{
    int p_pos;

    if (available_pos.empty())
    {
        p_pos = max_pollfd_pos++;
    }
    else
    {
        p_pos = available_pos.front();
        available_pos.pop();
    }

    int fd = node->getFd();

    if (nonblock)
    {
        fcntl(fd, F_SETFL, O_NONBLOCK);
    }
    
    fds[p_pos].fd = fd;
    fds[p_pos].events = POLLIN;
    fds[p_pos].revents = 0; //allows to avoid errors while reconnection
    nodes.push_back(std::make_pair(std::move(p_pos), node));
}

EventLoop::nodeRetType EventLoop::rmNode(nodeRetType &nodep)
{
    if (nodep->first == max_pollfd_pos - 1 && available_pos.empty())
        max_pollfd_pos--;
    else
        available_pos.push(nodep->first);

    fds[nodep->first].fd = -1; ///TODO remove it from vector
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
    //auto thread_pool = ThreadPool::getInstance(); TODO
    while (!stop)
    {
        
        int rc = poll(fds.data(), max_pollfd_pos, 0);
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
                    addNode(clnt, 1); 
                }
                if (!node->is_active())
                {
                    node_p = rmNode(node_p);
                }
            }
        }
        std::signal(SIGINT, EventLoop::handleSignal);
    }
    //thread_pool->stop(); TODO
}

void EventLoop::handleSignal(int signal)
{
    stop = signal;
}
