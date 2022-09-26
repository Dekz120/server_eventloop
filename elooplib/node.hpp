#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <fcntl.h>
#include <list>
#include <queue>

class serverExcept : public std::exception
{
public:
    serverExcept(){};
    serverExcept(std::string &&es) : err(errno), error_source(std::move(es)){};
    const char *what() const noexcept override
    {
        return error_source.c_str();
    }
    int err;
    std::string error_source;
};

class Node
{
private:
    size_t fd;
    bool active;

public:
    Node(size_t);
    Node();
    Node(Node &&);
    bool is_active();
    void closeConnection();
    size_t getFd();
    void setFd(int);
    virtual int handleConnection();
    virtual ~Node();
};
