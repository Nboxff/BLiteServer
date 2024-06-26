#pragma once
#include <map>
class EventLoop;
class Socket;
class Acceptor;
class Connection;

class Server {
private:
    EventLoop *loop;
    Acceptor *acceptor;
    std::map<int, Connection*> connections; // all TCP connections
     
public:
    Server(EventLoop *);
    ~Server();
    void newConnection(Socket *serv_sock);
    void deleteConnection(Socket *sock);
};