#ifndef ELOOP_INCLUDED
#define ELOOP_INCLUDED
#include <csignal>
#include <variant>
#include <vector>
#include <queue>
#include <list>
#include "node.hpp"
#include "server.hpp"
#include "client.hpp"

using Host = std::variant<Server, Client>;

static volatile std::sig_atomic_t stop;
struct signalHandler
{
    static void handleSignal(int signal)
        {
          stop = signal;
        }

};

class EventLoop
{
    private:
        int max_pollfd_pos;
        std::queue<int> available_pos;
        std::vector<pollfd> fds;
        std::list<std::pair<size_t, Host>> nodes; //pair <node, poll_position>
    public:
        EventLoop(size_t);
        void addNode(Host&&, bool);

        using nodeRetType = decltype(std::begin(nodes));
        nodeRetType rmNode(nodeRetType&);
        bool isActive(Host&);
        int handleConnection(Host&);
        void run();
};


#endif //ELOOP_INCLUDED
