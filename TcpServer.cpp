//
// Created by Yuchen Qin on 2021/10/18.
//

#include "TcpConnection.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "Acceptor.h"



TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, std::string name, bool reusePort):
        loop_(loop), ipPort_(listenAddr.toIpPort()), listenAddr_(listenAddr), name_(std::move(name)),
        acceptor_(new Acceptor(loop, listenAddr, reusePort)),
        threadPool_(new EventLoopThreadPool(loop)),
        connectionCallback_(defaultConnectionCallback),
        messageCallback_(defaultMessageCallback),
        nextConnId_(1),
        started_(false){
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
                                              std::placeholders::_1, std::placeholders::_2));

}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    for(auto& item : connections_){
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        //离开scope，自动析构
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads >= 0);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if(!started_){
        threadPool_->start(threadInitCallback_);

        assert(!acceptor_->listening());
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int connFd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    std::cout << "connFd: " << connFd << std::endl;

    EventLoop* ioLoop = threadPool_->getNextLoop();

    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    TcpConnectionPtr conn(new TcpConnection(ioLoop, connFd, listenAddr_, peerAddr, connName));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    auto res = connections_.erase(conn->name());
    assert(res == 1); // succes
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}