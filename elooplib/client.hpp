#pragma once

#include <regex>
#include "node.hpp"
#include "threadpool.hpp"
//#include "archive.hpp"

enum class requestType{none, time, echo, compression};

class Client : public Node
{
private:
    std::string request_field;
    std::string response;

    std::atomic<int> files_counter;
public:
    Client(int);
    Client(Client &&);
    int handleConnection() override;
    void recognizeData();
    void handleTime();
    void handleEcho();
    void handleCompress();
    void compressFile(const std::string&); //TODO
    
    void sendData();
};
