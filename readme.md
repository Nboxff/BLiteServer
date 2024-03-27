# BLiteServer
## 项目说明

改编自《30天自制C++服务器》。主要目的为复习C++高级程序设计。

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

