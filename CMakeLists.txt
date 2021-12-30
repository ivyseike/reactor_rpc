cmake_minimum_required(VERSION 3.20)
project(testP)

set(CMAKE_CXX_STANDARD 17)


add_executable(testP main.cpp ThreadSafeQueue.h ThreadPool.h ThreadJoiner.h CountdownLatch.h EventLoop.h EventLoop.cpp Channel.h Channel.cpp Poller.h Poller.cpp TimerFd.h TimerFd.cpp EventLoopThread.h EventLoopThread.cpp Acceptor.h Socket.h Socket.cpp InetAddress.h InetAddress.cpp Acceptor.cpp TcpServer.h EventLoopThreadPool.h EventLoopThreadPool.cpp TcpConnection.h TcpConnection.cpp TcpServer.cpp Connector.cpp Connector.h TcpClient.cpp TcpClient.h Codec.h RPC_Server.h traits.h RPC_Client.h)

