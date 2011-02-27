#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>

#ifdef WINDOWS
#define getpid() GetProcessId(NULL)
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#include "message.hpp"
#include "blockserver.hpp"
#include "blockclient.hpp"
#include "logger.hpp"




void Test()
{
    Msync::Message out(8768, 1, getpid(), "Hello, world!");
    Msync::BlockSocket socket("228.5.6.7", 9000);
    
    socket.open();
    socket.set_timeout(500);
    
    while (true) {
        Msync::Message in(1024);
        Msync::BlockSocket::Status status = socket.select(500);
       
        if (status == Msync::BlockSocket::READ || status == Msync::BlockSocket::BOTH) {
            socket >> in;
            std::cout << "Message: \"" << in.get_text() << "\"" << std::endl;
            std::cout << "PID: " << in.get_metadata<int>() << std::endl;
        }
        
        if (status == Msync::BlockSocket::WRITE || status == Msync::BlockSocket::BOTH) {
            socket << out;
            std::cout << "Sent message" << std::endl;
            sleep(1);
        }
    }
    socket.close();
}

void Listen()
{
    Msync::BlockSocket socket("228.5.6.7", 9000);
    Msync::Logger::Default.set_level(Msync::Logger::FINE);
    
    socket.open();
    socket.set_timeout(500);
    
    while (true) {
        
        Msync::BlockSocket::Status status = socket.select(-1);
       
        if (status == Msync::BlockSocket::READ || status == Msync::BlockSocket::BOTH) {
            
            Msync::Message in(4096);
            socket >> in;
            std::cout << "Message: " << in.get_type() << std::endl;
        }
    }
    socket.close();
}

int main(int argc, char** argv) 
{

    Msync::Logger::Default.set_level(Msync::Logger::FINE);
    try {
        if (argc == 3) {
            Msync::BlockServer server(argv[1], argv[2]);
            Msync::Logger::Default << Msync::Logger::INFO << "Starting server\n";
            server.start();
        } else if (argc == 1) {
            Msync::Logger::Default << Msync::Logger::INFO << "Starting client\n";
            Msync::BlockClient client;
            client.start();
        } else if (argc > 1 && std::string(argv[1]) == "listen") {
            Listen();
        }
    } catch (std::string& message)  {
        std::cout << message << std::endl;
    }

}
