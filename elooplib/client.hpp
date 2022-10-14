#pragma once

#include <regex>
#include "node.hpp"
#include "threadpool.hpp"
//#include "archive.hpp"

class Client : public Node
{
private:
    std::string request_field;
    std::string response;

public:
    Client(int);
    Client(Client &&);
    int handleConnection() override;
    size_t getFd() override;
    int recognizeData();
    int handleTime();
    int handleEcho();
    int handleCompress();
    
    int sendData();
    std::string getResponse();
};

class ClientTask : public Client
{
    public:
    ClientTask(int, Client&, std::shared_ptr<ThreadPool>&);
    ClientTask(ClientTask&& rhs);
    int handleConnection() override;
    void closeConnection();
    int compressFiles();
    size_t getFd() override;
    ~ClientTask();
    private:
    std::shared_ptr<ThreadPool> th_pool;
    std::string dir;
    int event_fd;
    std::atomic<int> complete_tasks{0};
    std::atomic<int> success_tasks{0};
};

class CompressITask : public ITask
{
    public:
    CompressITask(const std::string&, int, std::atomic<int>*, std::atomic<int>*, int);
    int compressFile();
    void run() override;
    private:
    std::string filename;
    int event_fd;
    std::atomic<int>* task_num;
    std::atomic<int>* success;
    int max;
    

};