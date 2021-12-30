//
// Created by Yuchen Qin on 2021/10/13.
//

#ifndef TESTP_TCPSERVER_H
#define TESTP_TCPSERVER_H

#include "TcpConnection.h"
#include "InetAddress.h"
#include <map>

class Acceptor;
class EventLoop;
class EventLoopThreadPool;


class TcpServer{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)> ;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, const char* buf, int len)> ;

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, std::string name, bool reusePort = true);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    EventLoop* getLoop() const { return loop_; }
    std::string ipPort() const { return ipPort_; }

    void setThreadNum(int numThreads);
    void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback & cb){ messageCallback_ = cb; }
    void setThreadInitCallback(const ThreadInitCallback& cb){ threadInitCallback_ = cb; }

    void start();




private:
    void newConnection(int connFd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_; //acceptor loop
    const std::string ipPort_;
    const std::string name_; // server name

    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_; // why shared?
    std::atomic_bool started_;
    int nextConnId_;
    ConnectionMap connections_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    ThreadInitCallback threadInitCallback_;

    const InetAddress listenAddr_;


};

#endif //TESTP_TCPSERVER_H

