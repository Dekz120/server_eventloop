#include "eventloop.hpp"

constexpr int max_clients = 50;

int main(int argc, char *argv[]) // 1th arg is a port
{

    if (argc != 2)
    {
        std::cerr << "argv must have exactly one argunent (TCP)" << std::endl;
        return -1;
    }
    try
    {  
        int tcp_port = std::stoi(argv[1]);
        auto srv = std::shared_ptr<Node>(new Server(tcp_port));
        EventLoop EL(max_clients);
        EL.addNode(srv, 1);
        EL.run();
    }
    catch (const serverExcept &se)
    {
        std::cerr << se.err << se.what() << std::endl;
        return -1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught : " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
