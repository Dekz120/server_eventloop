#include "client.hpp"
#include "threadpool.hpp"
#include <algorithm>
#include <filesystem>
#include <sys/eventfd.h>

Client::Client(int descr) : Node(descr) {}
Client::Client(Client &&rhs)
    : Node(static_cast<Node &&>(rhs)),
      request_field(std::move(rhs.request_field)),
      response(std::move(rhs.response)) {}

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
  int res = 0;
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

int Client::recognizeData()
{
  if (response.starts_with("time\n"))
  {
    return Client::handleTime();
  }

  if (response.starts_with("echo "))
  {
    return Client::handleEcho();
  }

  if (response.starts_with("compress "))
  {
    return Client::handleCompress();
  }
  else
  {
    response = "Wrong request\n";
    return 0;
  }
}

int Client::handleEcho()
{
  response = response.substr(5, request_field.size() - 5);
  return 0;
}

int Client::handleTime()
{
  auto t = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(t);
  response = std::ctime(&tt);
  return 0;
}

int Client::handleCompress()
{
  response = response.substr(9, request_field.size() - 10);
  const std::filesystem::path dir(response);
  if (!exists(dir))
  {
    response = "Path is not exists\n";
    return 0;
  }
  int fd = eventfd(0, EFD_CLOEXEC);
  return fd;
}
int Client::sendData()
{
  int r = recognizeData();
  if (r == 0)
  {
    int s = send(getFd(), response.c_str(), response.size(), 0);
    if (s < 0)
      throw serverExcept("send()");
    return r;
  }
  else // functions requires a threadpool
  {
    int s = send(getFd(), "Start compression...\n", 21, 0);
    if (s < 0)
      throw serverExcept("send()");
    return r;
  }
}

size_t Client::getFd() { Node::getFd(); };

std::string Client::getResponse() { return response; };
//////////
ClientTask::ClientTask(int fd, Client &cl, std::shared_ptr<ThreadPool> &tp)
    : Client(cl.getFd()), th_pool(tp), event_fd(fd)
{
  dir = cl.getResponse();
  ClientTask::compressFiles();
}

ClientTask::ClientTask(ClientTask &&rhs)
    : Client(static_cast<Client &&>(rhs)), th_pool(std::move(rhs.th_pool)) {}

int ClientTask::compressFiles()
{

  // TODO try_lock for directory name mutex
  int task_summ = 0;
  for (const auto &entry : std::filesystem::directory_iterator(dir))
    task_summ++;
  for (const auto &entry : std::filesystem::directory_iterator(dir))
  {
    std::string tmp = entry.path();
    if (!tmp.ends_with(".gz"))
    {
      auto ct = std::shared_ptr<ITask>(
          new CompressITask(tmp, ClientTask::getFd(), &complete_tasks,
                            &success_tasks, task_summ));
      th_pool->addTask(ct);
    }
  }
}

int ClientTask::handleConnection()
{
  dir = std::to_string(success_tasks) + "\n";
  int s = send(Node::getFd(), dir.c_str(), dir.size(), 0);
  if (s < 0)
    throw serverExcept("send()");
  int parent_fd = Node::getFd();
  ClientTask::closeConnection();
  return parent_fd;
}

void ClientTask::closeConnection()
{
  Node::closeConnection();
  setFd(-1); // needed to keep socket opened after destructor call
}

size_t ClientTask::getFd() { return event_fd; }
ClientTask::~ClientTask() { close(event_fd); }
/////////
CompressITask::CompressITask(const std::string &f, int fd, std::atomic<int> *tn,
                             std::atomic<int> *s, int m)
    : ITask(), filename(f), event_fd(fd), task_num(tn), success(s), max(m){};

void CompressITask::run() { compressFile(); }

int CompressITask::compressFile()
{
  // TODO compression
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(6000ms); // DEBUG

  *success += 1;
  *task_num += 1;
  if (*task_num == max)
  {
    int w = write(event_fd, "end compression", 15);
    if (w < 0)
      throw serverExcept("send()");
  }
  return 0;
}
