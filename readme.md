# BLiteServer
## 项目说明

改编自《30天自制C++服务器》，实验项目主要目的为复习C++高级程序设计，并了解Linux网络编程技术。

## 项目记录

### lab1 socket API 实现服务端和客户端

服务端：
- 创建socket
- 绑定IP地址和端口号(`bind`)
- 监听(`listen`)
- 接受一个客户端连接(`accpet`)

### lab2 echo服务器与错误处理
`perror`: Print a message describing the meaning of the value of errno （解释来自源码注释）

通过变量`errno`的值确定错误类型并报错

```cpp
void errif(bool condition, const char *errmsg){
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}
```
### lab3 epoll实现高并发

- 阻塞IO和非阻塞IO
- `select`、`poll`、`epoll`

> 阅读资料:
> 
> - [IO多路复用](https://mp.weixin.qq.com/s/YdIdoZ_yusVWza1PU7lWaw)
> - [epoll高效运行的原理](https://www.zhihu.com/tardis/zm/art/159135478?source_id=1005)

epoll是棵红黑树

### lab4 面向对象封装
C++的`class`声明结束要有一个`;`

避免头文件被重复引用
```cpp
#pragma once
```

避免头文件的循环依赖问题
```cpp
class InetAddress; // 前向声明用于解决循环依赖问题
```

调用全局作用域的函数，使用`::`
```cpp
void Socket::bind(InetAddress* inet_address) {
    int bind_ret = ::bind(fd, (sockaddr*)&inet_address->serv_addr, sizeof(inet_address->serv_addr));
    // ...
}
```

使用`delete`释放`new`出来的对象
```cpp
delete serv_sock, serv_addr, clnt_addr;
```
### lab5 Channel封装文件描述符

epoll中的`data`其实是一个union类型，可以储存一个指针，理论上我们可以指向任何一个地址块的内容，可以是一个类的对象，这样就可以将一个文件描述符封装成一个`Channel`类，一个`Channel`类自始至终只负责一个文件描述符，对不同的服务、不同的事件类型，都可以在类中进行不同的处理。

只在函数声明中指定**默认参数**的值

```cpp
std::vector<Channel*> Epoll::poll(int timeout) {
    int nfds = epoll_wait(this->epfd, events, MAX_EVENTS, timeout);
    // ...
}
```

### lab6 Reactor模式


> VSCode中写完头文件，快速在`cpp`文件中添加函数实现：`Ctrl + . + Enter`

> 阅读资料:
>
> - [如何深刻理解Reactor和Proactor](https://www.zhihu.com/question/26943938/answer/1856426252)

EventLoop就是Reactor模式中的main-Reactor

在创建`Channel`时传入处理事务的函数

**函数对象**

```cpp
void setCallback(std::function<void()> callback);
```

- 需要引入头文件：`#include <functional>`

- `std::function<void()>` 是一个模板类，用于表示可以调用的对象，例如函数、lambda 表达式或其他可调用的对象。在这里，它表示一个函数，该函数没有参数（即 () 中没有参数）且返回类型为 void。

- 调用时像函数一样直接调用: `callback();`

**std::bind 函数**

```cpp
std::function<void()> cb = std::bind(&Server::newConnection, this, serv_sock);
```
创建一个名为`cb`的可调用对象，该对象将在调用时执行` Server::newConnection`方法，并传递当前对象指针 `this` 和 `serv_sock` 参数。

**迪米特法则**

使用迪米特法则重构`Channel`类, Channel需要持有`EventLoop`对象的指针, 无需持有`Epoll`的指针

### lab7 Acceptor封装建立连接

分离"接受连接"这一模块

由`Acceptor`负责创建连接`Socket`, `Server`类负责管理`Acceptor`对象的生命周期

**std::placeholders::_1 占位符**

```cpp
std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
```

使用`std::bind`绑定一个成员函数时，有时需要将某些参数推迟到实际调用时再传递。`std::placeholders::_1`是一个特殊的占位符，它表示在绑定函数时的第一个参数位置。当我们调用绑定的函数时，`std::placeholders::_1`会被实际的参数值替换.

### lab8 封装TCP连接Connection

**类的组织与生命周期管理**

由`Server`类负责创建`Acceptor`和`Connection`，并管理这两个类的生命周期。所以`Server::newConnection`和`Server::deleteConnection`这两个函数写在`Server`类中，通过函数对象传入`Acceptor`和`Connection`。

`Server`类负责销毁`Acceptor`和`Connection`这两个类。

`Acceptor`和`Connection`负责销毁`Connection`和`Socket`，实现了对象的回收。

**debug记录**：忘记在初始化成员列表中初始化`EventLoop* loop, Socket* sock`。通过`printf`大法定位空指针问题。

```cpp
void Server::deleteConnection(Socket *sock) {
    printf("debug\n");
    Connection *conn = connections[sock->getFd()];
    printf("debug\n");
    connections.erase(sock->getFd());
    delete conn;
}
```
改进：多使用`assert()`
```cpp
void Server::deleteConnection(Socket *sock) {
    assert(sock != nullptr);
    Connection *conn = connections[sock->getFd()];
    connections.erase(sock->getFd());
    delete conn;
}
```


