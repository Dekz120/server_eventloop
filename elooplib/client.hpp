#pragma once

#include <regex>
#include <algorithm>
#include <filesystem>
#include <sys/eventfd.h>
#include "node.hpp"
#include "threadpool.hpp"
#include "tp_tasks.hpp"

struct Command
{
    enum
    {
        Time,
        Echo,
        Compress,
        Decompress,
        NotExitsts
    } m_cmd;
    std::string m_arg;
    void clear()
    {
        m_cmd = Command::NotExitsts;
        m_arg.clear();
    };
};
class Client : public Node
{
private:
    std::string request_field;
    std::string response;
    Command cmd;

public:
    Client(int);
    Client(Client &&);
    std::shared_ptr<Node> handleConnection() override;
    bool updateRequestField(const char *, size_t len);
    size_t getFd() override;
    std::shared_ptr<Node> recognizeData();
    std::shared_ptr<Node> handleTime();
    std::shared_ptr<Node> handleEcho();
    std::shared_ptr<Node> handleFileTask();
    void parseCommand();
    Command getCommand() { return cmd; }

    std::shared_ptr<Node> sendData();
    std::string getResponse();
};

class ClientTask : public Client
{
public:
    ClientTask(int e_fd);
    ClientTask(ClientTask &&rhs);
    std::shared_ptr<Node> handleConnection() override;
    void closeConnection();
    void prepareFileTask();
    void attachData(std::shared_ptr<ThreadPool> &, std::shared_ptr<Client> &);
    void createTask(const std::string &, int, std::atomic<int> *,
                    std::atomic<int> *, int);
    size_t getFd() override;
    ~ClientTask();

private:
    std::shared_ptr<ThreadPool> th_pool;
    std::string dir;
    int event_fd;
    std::atomic<int> complete_tasks{0};
    std::atomic<int> success_tasks{0};
    std::shared_ptr<Node> parentClient;
};