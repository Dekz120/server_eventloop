#pragma once
#include <csignal>
#include <variant>
#include <vector>
#include <queue>
#include <list>
#include <memory>
#include "node.hpp"
#include "server.hpp"
#include "client.hpp"

namespace
{
    volatile std::sig_atomic_t stop;
}

class EventLoop
{
private:
    int max_pollfd_pos;
    std::queue<int> available_pos;
    std::vector<pollfd> fds;
    std::list<std::pair<size_t, std::shared_ptr<Node>>> nodes; // TODO no pair, just remove from vector
    std::list<std::pair<size_t, std::shared_ptr<Node>>> wait_q;
    std::shared_ptr<ThreadPool> th_pool;

public:
    EventLoop(size_t);
    void addNode(std::shared_ptr<Node> &, bool);
    using nodeType = decltype(*std::begin(nodes));
    using nodePointerType = decltype(std::begin(nodes));
    nodePointerType rmNode(nodePointerType &);
    int handleConnection(std::shared_ptr<Node> &);
    void run();
    static void handleSignal(int);
    
    nodePointerType waitersSearch(int);
    void blockClinet(nodePointerType&);
};
