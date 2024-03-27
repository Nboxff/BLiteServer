#pragma once

class InetAddress; // 前向声明用于解决循环依赖问题

class Socket {
private:
    int fd;
public:
    Socket();
    Socket(int fd);
    ~Socket();
    void bind(InetAddress* inet_address);
    void listen();
    int accept(InetAddress* clnt_addr);
    int getFd();
    void setnonblocking();
};

