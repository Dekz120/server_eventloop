#include "eventloop.hpp"

EventLoop::EventLoop(size_t n_clients) : max_pollfd_pos(0), fds(n_clients),
                                         nodes(n_clients, nullptr), th_pool(new ThreadPool(get_nprocs())){};

void EventLoop::addNode(std::shared_ptr<Node> &node, bool nonblock)
{
    int fd = node->getFd();

    if (nonblock)
    {
        fcntl(fd, F_SETFL, O_NONBLOCK);
    }

    fds[max_pollfd_pos].fd = fd;
    fds[max_pollfd_pos].events = POLLIN;
    fds[max_pollfd_pos].revents = 0; // allows to avoid errors while reconnection
    nodes[max_pollfd_pos] = node;
    max_pollfd_pos++;
}

int EventLoop::rmNode(int indx)
{
    nodes.erase(nodes.begin() + indx);
    fds.erase(fds.begin() + indx);
    max_pollfd_pos--;
    return --indx;
}

std::shared_ptr<Node> EventLoop::handleConnection(std::shared_ptr<Node> &hst)
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
    return nullptr;
}

void EventLoop::run()
{
    while (!stop)
    {

        int rc = poll(fds.data(), max_pollfd_pos, 0);
        if (rc < 0)
            throw serverExcept("poll()");

        for (size_t pos = 0; pos < max_pollfd_pos + 1; pos++)
        {
            if (fds[pos].revents == POLLIN)
            {
                auto new_node = handleConnection(nodes[pos]);

                if (new_node)
                {
                    auto new_ptr = std::dynamic_pointer_cast<ClientTask>(new_node);
                    if (new_ptr)
                    {
                        auto parent_ptr = std::dynamic_pointer_cast<Client>(nodes[pos]);
                        new_ptr->attachData(th_pool, parent_ptr);
                        rmNode(pos);
                    }
                    addNode(new_node, 1);
                }
                if (nodes[pos])
                {
                    if (!nodes[pos]->is_active())
                    {
                        rmNode(pos);
                    }
                }
            }
        }
        std::signal(SIGINT, EventLoop::handleSignal); // TODO called once
    }
    th_pool->stop();
}

void EventLoop::handleSignal(int signal)
{
    stop = signal;
}
