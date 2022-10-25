#include "client_task.hpp"

ClientTask::ClientTask(int fd, Client &cl, std::shared_ptr<ThreadPool> &tp)
    : Client(cl.getFd()), th_pool(tp), event_fd(fd)
{
  dir = cl.getResponse();
  ClientTask::prepareFileTask();
}

ClientTask::ClientTask(ClientTask &&rhs)
    : Client(static_cast<Client &&>(rhs)), th_pool(std::move(rhs.th_pool)) {}

void ClientTask::prepareFileTask()
{
  std::function<bool(const std::string)> cond_func;
  int command;

  if (dir.starts_with("compress "))
  {
    dir = dir.substr(9, dir.size() - 10);
    cond_func = [](const std::string &f_name)
    { return !f_name.ends_with(".gz"); };
    command = 0;
  }
  else
  {
    dir = dir.substr((11), dir.size() - 12);
    cond_func = [](const std::string &f_name)
    { return f_name.ends_with(".gz"); };
    command = 1;
  }
  int task_summ = 0;
  for (const auto &entry : std::filesystem::directory_iterator(dir))
  {
    std::string tmp = entry.path();
    if (cond_func(tmp))
      task_summ++;
  }
  if (task_summ == 0)
  {
    int w = write(ClientTask::getFd(), "end compression", 15);
    if (w < 0)
      throw serverExcept("send()");
    return;
  }
  for (const auto &entry : std::filesystem::directory_iterator(dir))
  {
    std::string tmp = entry.path();
    if (cond_func(tmp))
    {
      std::shared_ptr<ITask> ct;
      switch (command) // TODO make command as enum class
      {
      case 0:
        ct = std::shared_ptr<ITask>(
            new CompressITask(tmp, ClientTask::getFd(), &complete_tasks,
                              &success_tasks, task_summ));
        break;
      case 1:
        ct = std::shared_ptr<ITask>(
            new DecompressITask(tmp, ClientTask::getFd(), &complete_tasks,
                                &success_tasks, task_summ));
        break;
      default:
        ct == nullptr;
        break;
      }
      if (ct)
        th_pool->addTask(ct);
    }
  }
}

int ClientTask::handleConnection()
{
  std::string resp = std::to_string(success_tasks) + "\n";
  int s = send(Node::getFd(), resp.c_str(), resp.size(), 0);
  if (s < 0)
    throw serverExcept("send()");
  int parent_fd = Node::getFd();
  ClientTask::closeConnection();
  return parent_fd;
}

void ClientTask::closeConnection()
{
  Node::closeConnection();
  setFd(-1); // need to keep socket opened after destructor call
}

size_t ClientTask::getFd() { return event_fd; }
ClientTask::~ClientTask() { close(event_fd); }