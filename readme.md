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
至此，一个完整的单线程服务器已经完成。

---

### lab9 缓冲区与线程池

> 这里将《30天自制C++服务器》的day9和day10合并为一个实验

为了实现在所有数据全部读入后，通过`echo`服务器一起返回，需要添加一个容量足够大的缓冲区。


疑难问题：
```cpp
class Connection {
private:
    // ...
    Buffer* readBuffer; // 在Connection类的构造函数中创建
};

// 和下面这种方法，优缺点是什么
class Connection {
private:
    // ...
    Buffer readBuffer;
};
```

每一个Reactor**只应该负责事件分发而不应该负责事件处理**，我们将构建一个最简单的线程池用于事件处理。

构建好线程池后，`Channel`不在直接调用函数进行事件处理，而是加入线程池的事件队列中，由线程池进行调度。

#### C++11 并发编程

1. 互斥锁 `std::mutex`
    
    确保在多线程环境下只有一个线程能够访问这些资源.
    ```cpp
    std::mutex mtx;
    mtx.lock();
    // do something
    mtx.unlock();

    ```
    使用`std::lock_guard<std::mutex> lock(mtx)`可以自动管理锁的生命周期。 

    ```cpp
    {
        std::lock_guard<std::mutex> lock(mtx);
        // 在这里执行临界区代码（critical section）
    } // 锁会在这里自动释放
    ``` 

    更灵活的锁定和解锁操作，可以使用 `std::unique_lock`：
    ```cpp
    {
        std::unique_lock<std::mutex> unique_lock(mtx);
        // 在这里执行临界区代码
        // 可以手动控制锁的获取和释放
    }
    ```

2. 创建线程 `std::thread`
    > 引用 [cppreference](https://en.cppreference.com/w/cpp/thread/mutex/lock) 中的示例代码，实现互斥的临界区变量++
    ```cpp
    int g_num = 0; // protected by g_num_mutex
    std::mutex g_num_mutex;
    
    void slow_increment(int id) {
        for (int i = 0; i < 3; ++i) {
            g_num_mutex.lock(); 
            ++g_num;
            // note, that the mutex also syncronizes the output
            std::cout << "id: " << id << ", g_num: " << g_num << '\n';
            g_num_mutex.unlock();
    
            std::this_thread::sleep_for(std::chrono::milliseconds(234));
        }
    }
    
    int main() {
        std::thread t1{slow_increment, 0};
        std::thread t2{slow_increment, 1};
        t1.join();
        t2.join();
    }
    ```

3. 条件变量 `std::condition_variable `

    用于在多线程环境中等待某个条件满足后再继续执行。
    创建一个 `std::condition_variable` 对象：
    ```cpp
    std::condition_variable cv;
    ```

    在一个线程中等待条件满足：
    ```cpp
    std::unique_lock<std::mutex> unique_lock(mtx);
    cv.wait(unique_lock, []{ return condition_is_met; });
    // 等待条件满足后继续执行
    ```
    在另一个线程中通知条件已满足：
    ```cpp
    {
        std::lock_guard<std::mutex> lock(mtx);
        condition_is_met = true;
    }
    cv.notify_one(); // 或者使用 cv.notify_all() 来通知所有等待的线程
    ```

#### C++11 std::vector高级用法

`push_back()`

- push_back() 方法用于将元素添加到容器（例如 std::vector）的末尾。

- 它接受一个现有对象（通过引用或右值引用）并将其复制（或移动）到容器中。

- 如果容器由于容量限制而需要重新分配内存，迭代器和对元素的引用可能会失效。

`emplace_back()`

- emplace_back() 是 C++11 中引入的功能，它直接在容器的末尾就地构造一个元素。

- 它避免了创建临时对象，然后将其复制或移动到容器中。

- 传递给 emplace_back() 的参数将转发到元素的构造函数。

- 如果发生重新分配，迭代器和引用可能会失效。

> 当现有对象要添加到容器时，请使用 `push_back()`。若想要直接就地构造一个新的临时对象时，使用 `emplace_back()`

#### lambda 表达式

lambda表达式完整形式：
```cpp
[捕获列表](函数参数) mutable throwSpec -> returnType { 函数体 }
```

无需捕获参数，无函数参数的例子：
```cpp
    auto lambda = [] {  // C++14 之后可以使用auto
        cout << "Hello, world!\n";
    };
    lambda(); // 调用函数
```

带参数的lambda表达式：
```cpp
    auto add = [](int x, int y) -> int{
        return x + y;
    };
    int c = add(3, 5); // 调用函数
```

捕获列表：
```cpp
    int x = 48; // 局部变量
    auto lambda = [x]() {
        cout << x << endl;
    }
    lambda();
```

使用`mutable`修饰符可以在函数体中修改`x`的值，但由于`x`是按值传递，不会影响局部变量`x`的值。若想修改局部变量的值，需要使用按引用传递：

```cpp
    int x = 48; // 局部变量
    auto lambda = [&x]() {
        x += 10;
    }
    lambda();
```

捕获所有的变量：

- `[=]`：按值捕获所有局部变量。

- `[&]`：按引用捕获所有局部变量。

- `[this]`：在成员函数中，可以直接捕获this指针。

#### 为线程池添加单元测试

打印不同颜色的字符
```cpp
#define ANSI_GREEN "\033[92m"
#define ANSI_RED "\033[91m"
#define ANSI_RESET "\033[0m"
```