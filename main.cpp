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
    serverExcept(string es) : err(errno), error_source(es){};
    static inline auto exceptionCheck(int v, string err_src)
        {
            if(v < 0)
                throw serverExcept(err_src);
        }
    int err;
    string error_source;
};

class Client{
public:
    Client(int d) : sd(d) {}
    const auto getPollData() const
    {
        return pair<int,int>(sd, poll_pos);
    }
    Client(Client&& rhs) : sd(rhs.sd), poll_pos(rhs.poll_pos), request(rhs.request)
    {
        rhs.sd = -1;
    }
    void setPollPos(int p)
    {
        poll_pos = p;
    }
    int runSession() // accept data
    {
        char buff[512];
        int rcv;

        rcv = recv(sd, buff, 512, 0);

        if(rcv == 0 ) //
        {
            return sd;
        }
        if(rcv > 0)
        {
            string req;
            req.resize(rcv);
            copy(buff, buff+rcv, begin(req));
            request += req;

            if(*--request.end() == '\n')
            {

                sendResponse(request);
                request.clear();
            }
        }
        return 0;

    }
    void sendResponse(string req)
    {
        int s;
        if(req.substr(0, 4) == "time")
        {
            auto t = chrono::system_clock::now();
            auto tt = chrono::system_clock::to_time_t(t);
            string resp = std::ctime(&tt);
            s = send(sd, resp.c_str(), resp.size(), 0);
            serverExcept::exceptionCheck(s, "send()");

        }
        else if(req.substr(0, 4) == "echo")
        {
            string tmp = req.substr(5, req.size()-5);
            int s = send(sd, tmp.c_str(), tmp.size(), 0);
            serverExcept::exceptionCheck(s, "send()");
        }

    }
    ~Client()
    {
        if(sd != -1) close(sd);
    }
private:
    int sd;
    int poll_pos; // allow to evade dscrptor in poll
    string request;
};

class Server{
public:
    Server(int port, int q) : con_queue(q), poll_pos(-1)
    {
        ld = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        serverExcept::exceptionCheck(ld, "socket()");

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;

		int bnd = bind(ld, (struct sockaddr*)&addr, sizeof(addr));
		serverExcept::exceptionCheck(bnd, "bind()");

		int lsn = listen(ld, con_queue);
		serverExcept::exceptionCheck(lsn, "listen()");
    }
    Server(Server&& rhs) : con_queue(rhs.con_queue), poll_pos(rhs.poll_pos),
        addr(rhs.addr), ld(rhs.ld)
    {
        rhs.ld = -1;
    }
    Client acceptCon() //getConnData
    {

        int accept_sd = accept(ld, (struct sockaddr*)&addr, (socklen_t*)&addr);
        serverExcept::exceptionCheck(accept_sd, "accept()");

        return(Client(accept_sd));

    }
    const auto getPollData() const
    {
        return pair<int,int>(ld, poll_pos);
    }

    void setPollPos(int p)//i still dont use it
    {
        poll_pos = p;
    }

    ~Server()
    {
        if(ld != -1) close(ld);
    }
private:
    int con_queue;
    int poll_pos;
    int ld;
    sockaddr_in addr;
};

class EventLoop
{
public:
    EventLoop(Server&& s, int clients_n) : srv(move(s)), fds(clients_n), max_pollfd_pos(0){};

    template<typename T>
    void addPollinDscr(T& role) // client or server
    {
        int p_pos; //position in pollfd vector
        auto [sd, nul] = role.getPollData();
        if(available_pos.empty())
        {
             p_pos = max_pollfd_pos++;
        }
        else
        {
            p_pos = available_pos.front();
            available_pos.pop();
        }

        role.setPollPos(p_pos);

        int flags = fcntl(sd, F_GETFL, 0);
        flags = (flags | O_NONBLOCK);
        fcntl(sd, F_SETFL, flags);

        fds[p_pos].fd = sd;
        fds[p_pos].events = POLLIN;

    }
    template<typename T>
	auto rmvConnection(T& iter_role)
	{
        auto [dsct, pos] = iter_role->getPollData();
        cout << pos << fds[pos].fd << " Removed\n";
		fds[pos].fd = -1;

		auto ret = clnts.erase(iter_role);
        if(pos == max_pollfd_pos-1 && available_pos.empty())
            max_pollfd_pos--;
        else
            available_pos.push(pos);

		return --ret;
	}
    void run()
    {
        addPollinDscr(srv);
        while(1)
        {
            int rc = poll(fds.data(), max_pollfd_pos, 0);
            if(rc < 0)
                {}
            if(fds[0].revents == POLLIN)
            {
                Client nclnt = srv.acceptCon();
                addPollinDscr(nclnt);

                clnts.push_back(move(nclnt));
            }

            for(auto cl_iter = clnts.begin(); cl_iter != clnts.end(); cl_iter++)
            {
                    auto [dscr, p_pos] = cl_iter->getPollData();

                    if(fds[p_pos].revents == POLLIN)
                    {
                        int ss = cl_iter->runSession();
                        if(ss > 0)
                        {
                            cl_iter = rmvConnection(cl_iter);
                        }
                    }
            }
        }
    }
private:
    int max_pollfd_pos;
    queue<int> available_pos;
    vector<pollfd> fds;
    Server srv;
    list<Client> clnts;
};

int main()
{

    try
    {
    Server srv(25444, 50);

    EventLoop ELOOPA(move(srv), 8);
    ELOOPA.run();
    }
    catch(const serverExcept& se)
    {
        cout << se.err << se.error_source << endl;
        close(0);
        return(-1);
    }
    return 0;
}
