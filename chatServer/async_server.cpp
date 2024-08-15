#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include "../networkLibrary/networkLibrary.h"
#include "../utils/Logger.h"

int main(int argc, char* argv[]){
    unsigned int port;
    try{
        if(argc != 2) throw;
        port = stoi(std::string(argv[1]));
    }
    catch(...){
        std::cout << "Incorrect Format" << std::endl;
        std::cout << "Required - <Server> <Port>" << std::endl;
        return 0;
    }
    
    boost::asio::io_context io_context;
    networkLibrary::Server::asyncServer server(io_context, port);
    io_context.run();

    return 0;
}