#include "eventloop.hpp"


int main(int argc, char* argv[]) // 1th arg is a port
{

    if(argc != 2)
    {
        std::cerr << "argv shoud've one argunent (TCP)"  << std::endl;
        return -1;
    }
    try
    {
    int tcp_port = atoi(argv[1]);
    Server srv(tcp_port);

    EventLoop EL(50);
    EL.addNode(std::move(srv), 1);
    EL.run();
    }
    catch(const serverExcept& se)
    {
        std::cerr << se.err << se.what() << std::endl;
        return(-1);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught : " << e.what() << std::endl;
        return -1;

    }
    return 0;
}
