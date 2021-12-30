//
// Created by Yuchen Qin on 2021/10/13.
//

#include <unistd.h>
#include "TcpConnection.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Socket.h"

void defaultConnectionCallback(const TcpConnection::TcpConnectionPtr& conn){
    std::cout << conn->localAddr_.toIpPort() << " -> "
            << conn->peerAddr_.toIpPort()
            << " is " << (conn->connected() ? "connected" : "connecting" ) << std::endl;
}

void defaultMessageCallback(const TcpConnection::TcpConnectionPtr& conn, const char* buf, int len){
    std::string mess(buf);
    if(mess.size() != len) mess.resize(len);
    std::cout << "Received message from: " << conn->peerAddr_.toIpPort()
            << " " << mess << std::endl;
}


TcpConnection::TcpConnection(EventLoop* loop, int sockfd, const InetAddress& localAddr,
                             const InetAddress& peerAddr, std::string name):
    loop_(loop), state_(kConnecting), socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr), peerAddr_(peerAddr), name_(std::move(name)){
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

void TcpConnection::handleRead() {
    char buf[65536];
    auto bytes_read = ::read(channel_->fd(), buf, sizeof(buf)); //能读多少读多少
    if(bytes_read > 0){
        if(messageCallback_)
            messageCallback_(shared_from_this(), buf, bytes_read);
    }
    else if(bytes_read == 0){
        handleClose();
    }
    else{
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if(channel_->isWriting() && !outputBuffer_.empty()){
        auto bytes_wrote = ::write(channel_->fd(), outputBuffer_.c_str(), outputBuffer_.size());
        if(bytes_wrote > 0){
            outputBuffer_.erase(0, bytes_wrote);
            if(outputBuffer_.empty()){
                channel_->disableWriting();
                if(state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }
        }
        else{
            std::cerr << "Error when writing" << std::endl;
        }
    }
}

void TcpConnection::handleClose(){
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    channel_->disableAll();
    setState(kDisconnected);
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError(){
    std::cerr << "TcpConnection: " << name_ << " error" << std::endl;
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if(state_ == kConnected || state_ == kDisconnecting){
        setState(kDisconnected);
        channel_->disableAll();
        closeCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::shutdown() {
    if(state_ == kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if(!channel_->isWriting()){
        socket_->shutdownWrite();
    }
}

void TcpConnection::send(const std::string& message) {
    if(state_ == kConnected){
        if(loop_->isInLoopThread()) sendInLoop(message);
        else loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), message));
    }
}

void TcpConnection::send(const char* message, int len) {
    if(state_ == kConnected){
        if(loop_->isInLoopThread()) sendInLoopChar(message, len);
        else loop_->runInLoop(std::bind(&TcpConnection::sendInLoopChar, shared_from_this(), message, len));
    }
}


void TcpConnection::sendInLoopChar(const char* message, int len) {
    loop_->assertInLoopThread();
    ssize_t bytes_wrote = 0;
    if(!channel_->isWriting() && outputBuffer_.empty()){
        bytes_wrote = ::write(channel_->fd(), message, len);
        if(bytes_wrote < 0){
            bytes_wrote = 0;
            std::cerr << "Write error" << std::endl;
        }
    }
    assert(bytes_wrote >= 0);
    if(bytes_wrote < len){
        std::string new_message(message);
        outputBuffer_.append(new_message.begin()+bytes_wrote, new_message.end());
        if(!channel_->isWriting()){
            channel_->enableWriting();
        }
    }
}
void TcpConnection::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    ssize_t bytes_wrote = 0;
    if(!channel_->isWriting() && outputBuffer_.empty()){
        bytes_wrote = ::write(channel_->fd(), message.c_str(), message.size());
        if(bytes_wrote < 0){
            bytes_wrote = 0;
            std::cerr << "Write error" << std::endl;
        }
    }
    assert(bytes_wrote >= 0);
    if(bytes_wrote < message.size()){
        outputBuffer_.append(message.begin()+bytes_wrote, message.end());
        if(!channel_->isWriting()){
            channel_->enableWriting();
        }
    }
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}

void TcpConnection::setTcpKeepAlive(bool on) {
    socket_->setTcpKeepAlive(on);
}



