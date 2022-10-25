#pragma once

#include <regex>
#include <algorithm>
#include <filesystem>
#include <sys/eventfd.h>
#include "node.hpp"
#include "threadpool.hpp"
#include "tp_tasks.hpp"
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
    int handleFileTask();

    int sendData();
    std::string getResponse();
};