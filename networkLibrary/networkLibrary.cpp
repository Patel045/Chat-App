#include "networkLibrary.h"

/*
    Asynchronous Server
*/

networkLibrary::Server::asyncServer::asyncServer(boost::asio::io_context &io_context, unsigned int port_num)
    : m_io_context(io_context),
      m_port(port_num),
      m_acceptor(
          m_io_context,
          boost::asio::ip::tcp::endpoint(
              boost::asio::ip::tcp::v4(),
              m_port))
{
    // std::cout << "Sucessfully Created asyncServer object" << std::endl;
    std::cout << "asyncServer(TCP/IP) started listening on Port : " << m_port << std::endl;
    this->startAccept();
}

void networkLibrary::Server::asyncServer::startAccept()
{
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket _socket)
        {
            // std::cout << "(dbg) : looping async_accept" <<std::endl;
                if (!ec)
                {
                    std::make_shared<networkLibrary::chatSession>(std::move(_socket), *this)->start();
                }
                startAccept();
            });

}

void networkLibrary::Server::asyncServer::write_broadcast(std::string buf)
    // implement broadcast
{
    auto _buf = boost::asio::buffer(buf);
    for(auto& _session : m_chat_sessions){
            boost::asio::
                async_write(
                    _session->m_socket, 
                    _buf,
                    [this,&_session](boost::system::error_code ec, int size){
                        if(ec){
                            std::cout << "Error Sending Data : " << ec.message() << std::endl;
                            m_chat_sessions.erase(_session);
                        }
                        else{
                            std::cout << "Broadcast -> Sent Message to " << _session->m_name << std::endl;
                        }       
        });
    }
}

/*
    Chat Session
*/

networkLibrary::chatSession::chatSession(boost::asio::ip::tcp::socket _socket, networkLibrary::Server::asyncServer& _serv)
    : m_socket(std::move(_socket)),
      m_ip(m_socket.remote_endpoint().address().to_string()),
      m_port(m_socket.remote_endpoint().port()),
      m_serv(_serv)
{

}

void networkLibrary::chatSession::start()
{
    m_serv.m_chat_sessions.insert(shared_from_this());

    auto self(shared_from_this());
    m_name = "New User";

    std::cout << "Client connected IP(" << m_ip << ":" << m_port << ")" << std::endl;

    // Ask username //maybe add passwords later
    m_buffer = "Server asks Username : \n";
    
    boost::asio::
        async_write(m_socket, boost::asio::dynamic_buffer(m_buffer),
                    [this](boost::system::error_code ec, int size)
                    {
        if(ec){
            std::cout << "Error Sending Data : " << ec.message() << std::endl;
            m_serv.m_chat_sessions.erase(shared_from_this());
        }
        else{
            std::cout << "Asked IP(" << m_ip << ":" << m_port << ") for Username" << std::endl;
            read_continous();
        } });
    m_buffer.clear();
}

void networkLibrary::chatSession::read_continous(){
    auto self(shared_from_this());
    // std::cout<<"Invoking async_read\n";
    boost::asio::
        async_read_until(
            m_socket, 
            boost::asio::dynamic_buffer(m_buffer), 
            "\n",
            [this, self](boost::system::error_code ec, int size)
        {
        if(ec){
            std::cout << "Error Sending Data : " << ec.message() << std::endl;
            m_serv.m_chat_sessions.erase(shared_from_this());
        }
        else{
            // std::cout << "Read done " <<std::endl;
            while(!m_buffer.empty() && m_buffer.back()=='\n') m_buffer.pop_back();
            if(m_name == "New User"){
                m_name = m_buffer;
                std::cout << "IP(" << m_ip << ":" << m_port << ") -> Username : " << m_name << std::endl;
                m_serv.write_broadcast(std::string (m_name+" joined the Server"));
            }
            else{
                m_buffer = m_name + " : " + m_buffer;
                std::cout << m_buffer << std::endl;
                m_serv.write_broadcast(m_buffer);
            }
            m_buffer.clear();
            read_continous();
        } });
}

networkLibrary::chatSession::~chatSession()
{
    std::cout << "Disconnecting IP(" << m_ip << ":" << m_port << ")" << std::endl;
}

/*
    Asynchronous Client
*/

networkLibrary::Client::asyncClient::asyncClient(boost::asio::io_context& _io_context , std::string _IPAddress, unsigned int _port)
    :m_io_context(_io_context),
    m_ip(_IPAddress),
    m_port(_port),
    m_socket(m_io_context),
    m_resolver(m_io_context)
{
    // std::cout << "Before Invoking async_connect" << std::endl;
    boost::asio::async_connect(
        m_socket,
        m_resolver.resolve(m_ip,boost::lexical_cast<std::string>(m_port)),
        [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint _endpoint){
            // std::cout << "Inside async_connect lambda" << std::endl;
            if(ec){
               std::cout << "Error Occured while Connecting to the Server" << std::endl;
            }else{
                read_continous();
            }
        });
}

void networkLibrary::Client::asyncClient::read_continous()
{
    // std::cout << "Invoking async_read_until" <<std::endl;
    boost::asio::async_read_until(
        m_socket,
        boost::asio::dynamic_buffer(m_buffer),
        "\n",
        [this](boost::system::error_code ec, int length){
            // std::cout << "Inside lambda of async_read_until" << std::endl;
            if(ec){
                std::cout << "Error in Reading from Server" <<std::endl;
                m_socket.close();
            }
            else{
                // std::cout << m_buffer.data() << std::endl;
                std::cout.write(m_buffer.data(), length);
                // m_buffer.clear();
                m_buffer.erase(0, length);
                read_continous();
            }
        }   
    );
}

void networkLibrary::Client::asyncClient::close_connection()
{
    boost::asio::post(
        m_io_context,
        [this](){
            m_socket.close();
            std::cout << "Connection Closed" << std::endl;
        }
    );
}

void networkLibrary::Client::asyncClient::write(std::string& buf)
{
    boost::asio::post(
        m_io_context,
        [this, buf](){
            std::string buffer;
            if(buf.back() != '\n'){
                buffer = buf + '\n';
            }
            else{
                buffer = buf;
            }
            boost::asio::async_write(
                m_socket,
                boost::asio::buffer(buffer),
                [this](boost::system::error_code ec, int length){
                    if(ec){
                        std::cout << "Error writing to Server" <<std::endl;
                        m_socket.close();
                    }
                    else{
                        // Successfully Sent
                        // std::cout << "Sent msg to Server" << std::endl;
                    }
                });
        });
}