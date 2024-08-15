#ifndef NETWORK_LIBRARY_H
#define NETWORK_LIBRARY_H

#include <iostream>
#include <string>
#include <set>
#include <memory>
#include <boost/asio.hpp>
#include <unistd.h>
// #include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace networkLibrary
{
    class chatSession;
    namespace Server
    {
        class asyncServer;
    };
    namespace Client
    {
        class asyncClient;
    };
};

class networkLibrary::Server::asyncServer
{
private:
    unsigned int m_port;
    boost::asio::io_context &m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    void startAccept();
    std::set<std::shared_ptr<chatSession>> m_chat_sessions;
public:
    friend class networkLibrary::chatSession;
    asyncServer(boost::asio::io_context &, unsigned int);
    void write_broadcast(std::string);
    // void deliver(const std::shared_ptr<networkLibrary::chatSession>&, boost::asio::mutable_buffers_1&);
};

class networkLibrary::Client::asyncClient
{
private:    
    boost::asio::io_context &m_io_context;
    std::string m_ip;
    unsigned int m_port;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver m_resolver;
    std::string m_buffer;
    void read_continous();
public:
    std::string m_username;
    asyncClient(boost::asio::io_context&, std::string, unsigned int);
    void write(std::string&);
    void close_connection();
};

class networkLibrary::chatSession : public std::enable_shared_from_this<networkLibrary::chatSession>
{
private:
    networkLibrary::Server::asyncServer &m_serv;
    std::string m_buffer;
    boost::asio::ip::tcp::socket m_socket;
    std::string m_ip;
    unsigned int m_port;
public:
    friend class networkLibrary::Server::asyncServer;
    chatSession(boost::asio::ip::tcp::socket, networkLibrary::Server::asyncServer&);
    ~chatSession();
    void start();
    void read_continous();
    std::string m_name;
};

#endif