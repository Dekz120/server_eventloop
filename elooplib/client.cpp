#include <filesystem>
#include <algorithm>
#include "client.hpp"
#include "threadpool.hpp"

Client::Client(int descr) : Node(descr){}
Client::Client(Client &&rhs) : Node(static_cast<Node &&>(rhs)),
                               request_field(std::move(rhs.request_field)) {}

int Client::handleConnection()
{
    char buff[512];
    size_t rcv;

    rcv = recv(getFd(), buff, 512, 0);
    if (rcv < 0)
    {
        return -1;
    }
    if (rcv == 0)
    {
        Node::closeConnection();
        return 0;
    }
    if (rcv > 0)
    {
        request_field.append(buff, rcv); 
        auto p = request_field.find('\n');
        if (p != std::string::npos)
        {

            response = request_field.substr(0, p + 1);
            sendData();
            request_field.clear();
        }
    }
    return 0;
}

void Client::recognizeData()
{
    std::string filter{" \n"};

    auto pos = response.find_first_of(filter);
    std::string command = response.substr(0, pos + 1);

    if (command == "time\n")
    {
        Client::handleTime();
        return;
    }

    if (command == "echo ")
    {
        Client::handleEcho();
        return;
    }

    if (command == "compress ")
    {
        Client::handleCompress();
        return;
    }
    else
    {
        response = "Wrong request\n";
    }
}

void Client::handleEcho()
{
    response = response.substr(5, request_field.size() - 5);
}

void Client::handleTime()
{
    auto t = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(t);
    response = std::ctime(&tt);
}

void Client::handleCompress()
{
    response = response.substr(9, request_field.size() - 10);
    std::cout << response << std::endl; // DEBUG
    const std::filesystem::path dir(response);
    if (!exists(dir))
    {
        response = "Path is not exists\n";
        return;
    }

    int dir_len = 0;
    // TODO try_lock if(1) etc
    std::list<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(response))
    {
        std::string tmp = entry.path();
        if(tmp.find(".gz") == std::string::npos)
        {
            //std::remove_if(begin(tmp), end(tmp), '\"'); TODO
            files.push_back(tmp); 

        }

        dir_len++;
    }
    
    files_counter = 0;
    for(const auto &f: files)
        compressFile(f);

    response = std::to_string(files_counter);
}

void Client::compressFile(const std::string &in_file) // TODO
{
    /*int res = compress(in_file.c_str(), (in_file + ".gz").c_str(),
                       6);
    if (res == Z_OK)
        files_counter++; */
}
void Client::sendData()
{
    recognizeData();
    int s = send(getFd(), response.c_str(), response.size(), 0);
    if (s < 0)
        throw serverExcept("send()");
}


