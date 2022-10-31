#include "client.hpp"

Client::Client(int descr) : Node(descr) {}
Client::Client(Client &&rhs)
    : Node(static_cast<Node &&>(rhs)),
      request_field(std::move(rhs.request_field)),
      response(std::move(rhs.response)) {}

std::shared_ptr<Node> Client::handleConnection()
{
  char buff[512];
  int rcv;

  rcv = recv(getFd(), buff, 512, 0);
  if (rcv < 0)
  {
    return nullptr;
  }
  if (rcv == 0)
  {
    Node::closeConnection();
    return nullptr;
  }
  std::shared_ptr<Node> res = nullptr;
  if (rcv > 0)
  {
    request_field.append(buff, rcv);
    auto p = request_field.find('\n');
    if (p != std::string::npos)
    {

      response = request_field.substr(0, p + 1);
      res = sendData();
      request_field.clear();
    }
  }
  return res;
}

std::shared_ptr<Node> Client::recognizeData() // Codes should be : OK or LONG_OPERATION IN TH_POOL
{
  if (response.starts_with("time\n"))
  {
    return Client::handleTime();
  }

  if (response.starts_with("echo "))
  {
    return Client::handleEcho();
  }

  if (response.starts_with("compress ") || response.starts_with("decompress "))
  {
    return Client::handleFileTask();
  }
  else
  {
    response = "Wrong request\n";
    return 0;
  }
}

std::shared_ptr<Node> Client::handleEcho()
{
  response = response.substr(5, request_field.size() - 5);
  return nullptr;
}

std::shared_ptr<Node> Client::handleTime()
{
  auto t = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(t);
  response = std::ctime(&tt);
  return nullptr;
}

std::shared_ptr<Node> Client::handleFileTask()
{
  std::string dir_name;
  if (response.starts_with("compress "))
    dir_name = response.substr(9, response.size() - 10);
  else
    dir_name = response.substr((11), response.size() - 12);
  const std::filesystem::path dir(dir_name);
  if (!exists(dir))
  {
    response = "Path is not exists\n";
    return 0;
  }
  int fd = eventfd(0, EFD_CLOEXEC);
  if (fd < 0) 
  {
    return nullptr;
  }
  else
    return std::shared_ptr<Node>(new ClientTask(fd));
}
std::shared_ptr<Node> Client::sendData()
{
  auto r = recognizeData();
  if (!r)
  {
    int s = send(getFd(), response.c_str(), response.size(), 0);
    if (s < 0)
      throw serverExcept("send()");
    return r;
  }
  else // functions requires a threadpool
  {
    int s = send(getFd(), "File task in progress...\n", 26, 0);
    if (s < 0)
      throw serverExcept("send()");
    return r;
  }
}

size_t Client::getFd() { return Node::getFd(); };

std::string Client::getResponse() { return response; };