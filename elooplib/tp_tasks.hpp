#pragma once

#include <sys/file.h>
#include <filesystem>
#include "threadpool.hpp"
#include "node.hpp"
#include "archive.hpp"

class FileITask : public ITask
{
public:
  FileITask(const std::string &, int, std::atomic<int> *, std::atomic<int> *, int);
  int handleFile(const std::string &, const std::string &,
                 std::function<int(FILE *, FILE *, int)> &);
  virtual int doWork(){return 0;};
  void run() override;
  void closeTask();

protected:
  std::string filename;
  int event_fd;
  std::atomic<int> *task_num; // TODO shared_ptr
  std::atomic<int> *success;
  int max;
};

class CompressITask : public FileITask
{
public:
  CompressITask(const std::string &, int, std::atomic<int> *, std::atomic<int> *, int);
  int doWork() override;
};

class DecompressITask : public FileITask
{
public:
  DecompressITask(const std::string &, int, std::atomic<int> *, std::atomic<int> *, int);
  int doWork() override;
};