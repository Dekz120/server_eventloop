#pragma once
#include <csignal>
#include <variant>
#include <vector>
#include <queue>
#include <list>
#include "node.hpp"
#include "server.hpp"
#include "client.hpp"
#include <sys/sysinfo.h>

namespace
{
    volatile std::sig_atomic_t stop;
}

class EventLoop
{
private:
    size_t max_pollfd_pos;
    std::queue<int> available_pos;
    std::vector<pollfd> fds;
    std::vector<std::shared_ptr<Node>> nodes; // TODO no pair, just remove from vector
    std::shared_ptr<ThreadPool> th_pool;

public:
    EventLoop(size_t);
    void addNode(std::shared_ptr<Node> &, bool);
    using nodeType = decltype(*std::begin(nodes));
    using nodePointerType = decltype(std::begin(nodes));
    int rmNode(int);
    std::shared_ptr<Node> handleConnection(std::shared_ptr<Node> &);
    void run();
    static void handleSignal(int);

    nodePointerType waitersSearch(int);
    void blockClinet(nodePointerType &);
};
