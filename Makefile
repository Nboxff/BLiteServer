server:
	g++ src/util.cpp client.cpp -o client && \
	g++ -pthread src/util.cpp server.cpp src/Epoll.cpp src/InetAddress.cpp src/Socket.cpp src/Channel.cpp src/EventLoop.cpp src/Server.cpp src/Acceptor.cpp src/Connection.cpp src/Buffer.cpp src/ThreadPool.cpp -o server

clean:
	rm server && rm client && rm test

threadPoolTest:
	g++ -pthread src/ThreadPool.cpp ThreadPoolTest.cpp -o test