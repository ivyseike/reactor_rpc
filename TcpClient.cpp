//
// Created by Yuchen Qin on 2021/11/1.
//

#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

void removeConnection(EventLoop* loop, const TcpClient::TcpConnectionPtr& conn){
    loop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

struct sockaddr_in getLocalAddr(int sockfd){
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, (struct sockaddr *)&localaddr, &addrlen) < 0){
        std::cerr << "sockets::getLocalAddr" << std::endl;
    }
    return localaddr;
}

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr, const std::string &nameArg)
    : loop_(loop), connector_(new Connector(loop, serverAddr)), name_(nameArg),
    connectionCallback_(defaultConnectionCallback), messageCallback_(defaultMessageCallback),
    retry_(false), connect_(false), nextConnId_(1), serverAddr_(serverAddr) {

    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

void TcpClient::connect() {
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;


    {
        std::lock_guard<std::mutex> lk(mutex_);
        if(connection_){
            connection_->shutdown();
        }
    }

}

void TcpClient::stop(){
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr = serverAddr_;
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    InetAddress localAddr(::getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_, sockfd, localAddr, serverAddr_, connName));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lk(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn){
    // closecallback， 连接的服务器close掉了
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        std::lock_guard<std::mutex> lk(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_){
        connector_->restart();
    }
}

