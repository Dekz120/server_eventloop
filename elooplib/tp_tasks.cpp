#include "tp_tasks.hpp"
#define SET_BINARY_MODE(file)

FileITask::FileITask(const std::string &f, int fd, std::atomic<int> *tn,
                     std::atomic<int> *s, int m)
    : filename(f), event_fd(fd), task_num(tn), success(s), max(m){};

void FileITask::run() { doWork(); }

void FileITask::closeTask()
{
  int w = write(event_fd, "end compression", 15);
  if (w < 0)
    throw serverExcept("send()");
}
int FileITask::handleFile(const std::string &src_name, const std::string &dst_name,
                          std::function<int(FILE *, FILE *, int)> &fileFunc)
{

  if (std::filesystem::exists(dst_name))
  {
    *task_num += 1;
    return -1;
  }
  FILE *src = fopen(src_name.c_str(), "r");
  FILE *dst = fopen(dst_name.c_str(), "w");

  if (!src || !dst)
  {
    *task_num += 1;
    fclose(src); // TODO errors
    fclose(dst);
    return -1;
  }
  int src_lock = flock(src->_fileno, LOCK_EX | LOCK_NB);
  int dst_lock = flock(dst->_fileno, LOCK_EX | LOCK_NB);

  int res = 0;
  if (src_lock == dst_lock && dst_lock == 0)
    res = fileFunc(src, dst, -1);
  else
    res = -1;

  //using namespace std::chrono_literals; // DEBUG
  //std::this_thread::sleep_for(6000ms); 

  std::filesystem::remove(src_name);
  flock(src->_fileno, LOCK_UN);
  flock(dst->_fileno, LOCK_UN);
  fclose(src);
  fclose(dst);

  *task_num += 1;
  if (*task_num == max) // TODO task_num could be desroyed outside. use shared ptr + new
    closeTask();
  if (res == 0)
    *success += 1;
  ////
  return res;
}
////
CompressITask::CompressITask(const std::string &f, int fd, std::atomic<int> *tn,
                             std::atomic<int> *s, int m) : FileITask(f, fd, tn, s, m) {}
int CompressITask::doWork()
{
  std::string dst_name = filename + ".gz";
  std::function<int(FILE *, FILE *, int)> compFunc = [](FILE *src, FILE *dst, int lvl)
  {
    return compress(src, dst, lvl);
  };
  int res = FileITask::handleFile(filename, dst_name, compFunc);
  return res;
}

DecompressITask::DecompressITask(const std::string &f, int fd, std::atomic<int> *tn,
                                 std::atomic<int> *s, int m) : FileITask(f, fd, tn, s, m) {}
int DecompressITask::doWork()
{
  std::string dst_name = filename.substr(0, filename.size() - 3);
  std::function<int(FILE *, FILE *, int)> compFunc = [](FILE *src, FILE *dst, int lvl)
  {
    return decompress(src, dst);
  };
  int res = FileITask::handleFile(filename, dst_name, compFunc);
  return res;
}