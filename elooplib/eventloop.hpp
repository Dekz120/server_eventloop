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
    std::list<std::pair<size_t, std::shared_ptr<Node>>> nodes; //TODO no pair, just remove from vector
    //ThreadPool th_pool; TODO
public:
    EventLoop(size_t);
    void addNode(std::shared_ptr<Node> &, bool);
    using nodeRetType = decltype(std::begin(nodes));
    nodeRetType rmNode(nodeRetType &);
    int handleConnection(std::shared_ptr<Node> &);
    void run();
    static void handleSignal(int);
};
