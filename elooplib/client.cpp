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
  std::shared_ptr<Node> res{nullptr};
  if (rcv > 0)
  {
    bool gotReq = updateRequestField(buff, rcv);
    if (gotReq)
    {
      parseCommand();
      res = sendData();
    }
  }
  return res;
}

bool Client::updateRequestField(const char *req, size_t len)
{
  request_field.append(req, len);
  auto p = request_field.find('\n');
  if (p != std::string::npos)
  {
    response = request_field.substr(0, p + 1);
    if (p == request_field.size() - 1)
      request_field.clear();
    else
      request_field = request_field.substr(p + 1, request_field.size() - p - 1);
    return 1;
  }
  else
    return 0;
}

void Client::parseCommand()
{
  if (response.starts_with("time\n"))
  {
    command.cmd = Command::Time;
    command.arg.clear();
    return;
  }

  if (response.starts_with("echo "))
  {
    command.cmd = Command::Echo;
    command.arg = response.substr(5, response.size() - 5);
    return;
  }

  if (response.starts_with("compress "))
  {
    command.cmd = Command::Compress;
    command.arg = response.substr(9, response.size() - 10);
    return;
  }

  if (response.starts_with("decompress "))
  {
    command.cmd = Command::Decompress;
    command.arg = response.substr((11), response.size() - 12);
    return;
  }

  else
  {
    command.cmd = Command::Unknown;
  }
}

std::shared_ptr<Node> Client::recognizeData() // Codes should be : OK or LONG_OPERATION IN TH_POOL
{
  if (command.cmd == Command::Time)
  {
    return Client::handleTime();
  }

  if (command.cmd == Command::Echo)
  {
    return Client::handleEcho();
  }

  if (command.cmd == Command::Compress || command.cmd == Command::Decompress)
  {
    return Client::handleFileTask();
  }
  else
  {
    response = "Wrong request\n";
  }
  return nullptr;
}

std::shared_ptr<Node> Client::handleEcho()
{
  response = command.arg;
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
  if (command.cmd == Command::Compress)
    dir_name = command.arg;
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
    command.clear();
    response.clear();
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