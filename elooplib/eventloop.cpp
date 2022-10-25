#include "eventloop.hpp"

EventLoop::EventLoop(size_t n_clients) : fds(n_clients), max_pollfd_pos(0),
                                         th_pool(new ThreadPool(3)){};

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
    // fds[p_pos].revents = 0; //allows to avoid errors while reconnection
    nodes.push_back(std::make_pair(std::move(p_pos), node));
}

EventLoop::nodePointerType EventLoop::rmNode(nodePointerType &nodep)
{
    if (nodep->first == max_pollfd_pos - 1 && available_pos.empty())
        max_pollfd_pos--;
    else
        available_pos.push(nodep->first);

    fds.erase(fds.begin() + nodep->first);

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

EventLoop::nodePointerType EventLoop::waitersSearch(int fd)
{
    auto nodeComp = [&fd](nodeType &n)
    {
        return fd == n.second->getFd();
    };
    auto wp = find_if(begin(wait_q), end(wait_q), nodeComp);
    return wp;
}

void EventLoop::blockClinet(nodePointerType &node_p)
{
    wait_q.push_back(std::move(*node_p));
    node_p = nodes.erase(node_p);
    --node_p;
}

void EventLoop::run()
{
    while (!stop)
    {

        int rc = poll(fds.data(), max_pollfd_pos, 0);
        if (rc < 0)
            throw serverExcept("poll()");

        for (nodePointerType node_p = nodes.begin(); node_p != nodes.end(); node_p++)
        {
            auto [pos, node] = *node_p;
            if (fds[pos].revents == POLLIN)
            {
                int data = handleConnection(node); // TODO WTF is data?

                if (data > 0) // TODO if need to use thread pool create eventFd(better in class)
                {             // then i need to redefine Server handleConnection
                    auto wp = waitersSearch(data);
                    if (wp != wait_q.end()) // There is a blocked Node where fd = data
                    {
                        nodes.push_back(std::move(*wp));
                        wait_q.erase(wp);
                    }
                    else
                    {
                        auto tmp_ptr = std::dynamic_pointer_cast<Client>(node);
                        if (tmp_ptr) // Client has returned new eventfd
                        {
                            auto clnt_task = std::shared_ptr<Node>(
                                new ClientTask(data, *tmp_ptr, th_pool));

                            addNode(clnt_task, 0);
                            blockClinet(node_p);
                            node = nullptr;
                        }
                        else // Server have got a new connection
                        {
                            auto clnt = std::shared_ptr<Node>(new Client(data));
                            addNode(clnt, 1);
                        }
                    }
                }
                if (node) // check wheter node was moved
                {
                    if (!node->is_active())
                        node_p = rmNode(node_p);
                }
            }
        }
        std::signal(SIGINT, EventLoop::handleSignal);
    }
    th_pool->stop();
}

void EventLoop::handleSignal(int signal)
{
    stop = signal;
}
