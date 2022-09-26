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
    int maxPollfdPos;
    std::queue<int> availablePos;
    std::vector<pollfd> fds;
    std::list<std::pair<size_t, std::shared_ptr<Node>>> nodes; // pair <node, poll_position>
public:
    EventLoop(size_t);
    void addNode(std::shared_ptr<Node> &, bool);
    using nodeRetType = decltype(std::begin(nodes));
    nodeRetType rmNode(nodeRetType &);
    int handleConnection(std::shared_ptr<Node> &);
    void run();
    static void handleSignal(int);
};
