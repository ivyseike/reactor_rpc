//
// Created by Yuchen Qin on 2021/10/12.
//

#include "Acceptor.h"
#include "EventLoop.h"
#include "Channel.h"
#include "InetAddress.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort): loop_(loop), listening_(false),
                acceptSocket_(socket(PF_INET, SOCK_STREAM, 0)), acceptChannel_(loop, acceptSocket_.fd()),
                idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)){
    assert(idleFd_ >= 0);
    if(reusePort){
        acceptSocket_.setReuseAddr(true);
    }
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
    std::cout << "Acceptor exiting..." << std::endl;
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen(); //转为listen状态
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    //listen fd上有数据来时会调用这个函数
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0){
        if(newConnectionCallback_){
            newConnectionCallback_(connfd, peerAddr);
        }
        else ::close(connfd);
    }
    else{
        std::cerr << "Error in Acceptor::handleRead" << std::endl;
    }

}