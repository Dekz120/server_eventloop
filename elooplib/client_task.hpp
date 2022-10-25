#include "client.hpp"

class ClientTask : public Client
{
public:
    ClientTask(int, Client &, std::shared_ptr<ThreadPool> &);
    ClientTask(ClientTask &&rhs);
    int handleConnection() override;
    void closeConnection();
    void prepareFileTask();
    void createTask(const std::string &, int, std::atomic<int> *,
                    std::atomic<int> *, int);
    size_t getFd() override;
    ~ClientTask();

private:
    std::shared_ptr<ThreadPool> th_pool;
    std::string dir;
    int event_fd;
    std::atomic<int> complete_tasks{0};
    std::atomic<int> success_tasks{0};
};