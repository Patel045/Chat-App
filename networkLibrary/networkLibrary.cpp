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
    std::cout << "asyncServer(TCP/IP) started listening on Port : " << m_port << std::endl;
    this->startAccept();
}

void networkLibrary::Server::asyncServer::startAccept()
{
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket _socket)
        {
                if (!ec)
                {
                    std::make_shared<networkLibrary::chatSession>(std::move(_socket), *this)->start();
                }
                startAccept();
            });

}

void networkLibrary::Server::asyncServer::write(const std::shared_ptr<networkLibrary::chatSession> _session, boost::asio::mutable_buffers_1 _buf)
{
    char* buffer_ptr = boost::asio::buffer_cast<char*>(_buf);   
    int sz = _buf.size();
    if(buffer_ptr[sz-1]!='\n') _buf += '\n';
    
    boost::asio::
        async_write(
            _session->m_socket,
            _buf, 
            [this, &_session](boost::system::error_code ec, int size){
                if(ec){
                    std::cout << "Error Sending Data : " << ec.message() << std::endl;
                    m_chat_sessions.erase(_session);
                }
                else{
                    // std::cout << "Broadcast -> Sent Message to " << _session->m_name << std::endl;
                }
    });
}

void networkLibrary::Server::asyncServer::write(const std::shared_ptr<networkLibrary::chatSession> _session, std::string buf)
{
    if(buf.back()!='\n') buf += '\n';
    boost::asio::mutable_buffers_1 _buf = boost::asio::buffer(buf);
    write(_session, _buf);
}


void networkLibrary::Server::asyncServer::write_broadcast(std::string buf)
{
    if(buf.back()!='\n') buf += '\n';
    auto _buf = boost::asio::buffer(buf);
    for(auto& _session : m_chat_sessions){
        write(_session, _buf);
    }
}

std::pair<std::string,std::string> networkLibrary::Server::asyncServer::parse(std::string &recv_message)
{   
    std::pair<std::string,std::string> _parsed_pr;
    std::string& name = _parsed_pr.first;
    std::string& message = _parsed_pr.second;

    int sz = recv_message.size();
    int cnt = 0;

    for(unsigned i=0; i<sz; ++i){
        if(cnt==0){
            if(recv_message[i] != ':') name += recv_message[i];
            else{
                cnt += 1;
                while(!name.empty() && (name.back()==' ' || name.back()=='\n')) name.pop_back();
            }
        }
        else{
            if(!(message.empty() && (recv_message[i]==' ' || recv_message[i]=='\n'))) message += recv_message[i];
        }
    }
    while(!message.empty() && (message.back()==' ' || message.back()=='\n')) message.pop_back();

    return _parsed_pr;
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
    boost::asio::
        async_read_until(
            m_socket, 
            boost::asio::dynamic_buffer(m_buffer), 
            "\n",
            [this, self](boost::system::error_code ec, int size)
        {
        if(ec){
            std::cout << "Error Reading Data : " << ec.message() << std::endl;
            m_serv.m_chat_sessions.erase(shared_from_this());
        }
        else{
            while(!m_buffer.empty() && m_buffer.back()=='\n') m_buffer.pop_back();
            if(m_name == "New User"){
                m_name = m_buffer;
                std::cout << "IP(" << m_ip << ":" << m_port << ") -> Username : " << m_name << std::endl;
                m_serv.write_broadcast(std::string (m_name+" joined the Server"));
            }
            else if(m_buffer[0]=='\\'){
                /*
                    Client Commands - 
                        1. \help
                        2. \list
                        3. \name_change
                        4. \msg
                        5. \quit
                */
                std::string message;
                std::cout << m_name << " asked for Special Command " << m_buffer << std::endl;
                
                if(m_buffer == "\\help"){
                    message = 
                    "Client Commands - \n"
                    "    1. \\help            : List out Client Commands\n"
                    "    2. \\list            : List of Connected Clients\n"
                    "    3. \\name_change {}  : Change your name\n"
                    "    4. \\msg {}{}        : Message Privately to some other Client\n"
                    "    5. \\quit            : Quit the Chat-Room\n"
                    "\n"
                    ;
                    std::shared_ptr<networkLibrary::chatSession> shared_session_ptr = self;
                    m_serv.write(shared_session_ptr, message);
                }
                else if(m_buffer == "\\list"){
                    message = "Clients in the Chat-Room:-\n";
                    int cnt = 0;
                    for(auto& _session: m_serv.m_chat_sessions){
                        message += std::string("    ") + std::to_string(++cnt) + std::string(". ");
                        message += _session->m_name;
                        message += '\n';
                    }
                    std::shared_ptr<networkLibrary::chatSession> shared_session_ptr = self;
                    m_serv.write(shared_session_ptr, message);
                }
                else if(m_buffer.substr(0,12) == "\\name_change"){
                    std::string _new_name;
                    bool in = false;
                    for(auto c: m_buffer){
                        if(c=='}') in =false;
                        if(in) _new_name += c;
                        if(c=='{') in = true;
                    }
                    message = std::string("Changed Name of ") 
                            + m_name 
                            + std::string(" to ") 
                            + _new_name;
                    m_name = _new_name;
                    std::cout << message <<std::endl;
                    m_serv.write_broadcast(message);
                } 
                else if(m_buffer.substr(0,4) == "\\msg"){
                    std::string _send_to;
                    std::string _message;
                    bool in = false;
                    int cntr = 0;
                    for(auto c: m_buffer){
                        if(c=='}'){
                            in =false;
                            ++cntr;
                        }
                        if(in && cntr==0) _send_to += c;
                        if(in && cntr==1) _message += c;
                        if(c=='{') in = true;
                    }
                    _message = m_name + " : " + _message; 
    
                    for(auto& _session: m_serv.m_chat_sessions){
                        if(_session->m_name == _send_to){
                            std::shared_ptr<networkLibrary::chatSession> shared_session_ptr = _session;
                            m_serv.write(shared_session_ptr, _message);
                            break;
                        }
                    }
                }
                else if(m_buffer == "\\quit"){
                    m_serv.m_chat_sessions.erase(self);
                    return;
                }
                else{
                    message = "Command Not Identified";
                    std::shared_ptr<networkLibrary::chatSession> shared_session_ptr = self;
                    m_serv.write(shared_session_ptr, message);
                }
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
    std::cout << "Disconnected IP(" << m_ip << ":" << m_port << ")" << " Username : " << m_name << std::endl;
    m_serv.write_broadcast(std::string("Disconnected "+m_name));
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
    boost::asio::async_connect(
        m_socket,
        m_resolver.resolve(m_ip,boost::lexical_cast<std::string>(m_port)),
        [this](boost::system::error_code ec, boost::asio::ip::tcp::endpoint _endpoint){
            if(ec){
               std::cout << "Error Occured while Connecting to the Server | Relaunch Client" << std::endl;
            }
            else{
                read_continous();
            }
        });
}

void networkLibrary::Client::asyncClient::read_continous()
{
    boost::asio::async_read_until(
        m_socket,
        boost::asio::dynamic_buffer(m_buffer),
        "\n",
        [this](boost::system::error_code ec, int length){
            if(ec){
                // std::cout << ec.message() << std::endl;
                close_connection();
                return;
            }
            else{
                while(m_buffer.back()=='\n') m_buffer.pop_back();
                std::cout << m_buffer.data() << std::endl;
                m_buffer.clear();
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
            std::cout << "Connection Socket Closed" << std::endl;
        }
    );
}

bool networkLibrary::Client::asyncClient::write(std::string& buf)
{
    std::string buffer = buf;
    while(!buffer.empty() && (buffer.back()=='\n' || buffer.back()==' ')) buffer.pop_back();
    reverse(buffer.begin(),buffer.end());
    while(!buffer.empty() && (buffer.back()=='\n' || buffer.back()==' ')) buffer.pop_back();
    reverse(buffer.begin(),buffer.end());
            
    if(buffer.empty()) return true;
    buffer += '\n'; 
    
    if(buffer == "\\quit\n") return (!write_now(buffer));
    
    boost::asio::post(
        m_io_context,
        [this, buffer](){
            boost::asio::async_write(
                m_socket,
                boost::asio::buffer(buffer),
                [this](boost::system::error_code ec, int length){
                    if(ec){
                        std::cout << "Error writing to Server | Relaunch Client" <<std::endl;
                        // m_socket.close();
                        close_connection();
                    }
                    else{
                        // Successfully Sent
                    }
                });
        });
    return true;    
}

bool networkLibrary::Client::asyncClient::write_now(std::string message)
{
    //blocking call
    boost::asio::write(
        m_socket,
        boost::asio::buffer(message)
    );
    return true;
}

networkLibrary::Client::asyncClient::~asyncClient()
{
    std::cout << "Client Closed" << std::endl;
}