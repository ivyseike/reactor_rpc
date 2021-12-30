//
// Created by Yuchen Qin on 2021/10/25.
//

#include <unistd.h>
#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr): loop_(loop), serverAddr_(serverAddr), connect_(false),
                    state_(kDisconnected), retryDelayMs_(kInitRetryDelayMs){ }

Connector::~Connector() {
   assert(channel_ == nullptr);
}

void Connector::start() {
    connect_ = true;
    loop_->runInLoop((std::bind(&Connector::startInLoop, this)));
}

void Connector::startInLoop() {
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if(connect_){
       connect();
    }
}

void Connector::connect()
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    auto sockaddr = serverAddr_.getSockAddr();
    int ret = ::connect(sockfd, sockaddr, static_cast<socklen_t>(sizeof(*sockaddr)));
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            std::cerr << "connect error in Connector::startInLoop " << savedErrno << std::endl;
            ::close(sockfd);
            break;

        default:
            std::cerr << "Unexpected error in Connector::startInLoop " << savedErrno << std::endl;
            ::close(sockfd);
            break;
    }
}

void Connector::connecting(int sockfd){
    setState(kConnecting);
    assert(channel_ == nullptr);

    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(std::bind(&Connector::handleError, this));

    channel_->enableWriting();

    //在connecting成功后不再进行connect操作
}

void Connector::stop(){
    connect_ = false;
    loop_->runInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop() {
    loop_->assertInLoopThread();
    if(state_ == kConnecting){
        setState(kDisconnected);
        int sockfd = removeAndResetChannel(); //reset means reset the pointer
        retry(sockfd); //因为stop把connect_设为false，进入retry只会关闭socket而不会试图再次重连
    }
}

int Connector::removeAndResetChannel() {
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->runInLoop(std::bind(&Connector::resetChannel, this));

    // Can't reset channel_ here, because we are inside Channel::handleEvent
    return sockfd;
}

void Connector::resetChannel() {
    channel_.reset();
}

void Connector::retry(int sockfd){
    ::close(sockfd);
    setState(kDisconnected);
    if(connect_){
        std::cout << "Retry connecting to " << serverAddr_.toIpPort() << " in " <<
                retryDelayMs_ << " milliseconds. ";
        sleep(retryDelayMs_ * 1000);
        loop_->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
}

void Connector::handleWrite() { //第一次可写事件后就不用Connector再进行交互了
    if(state_ == kConnecting){
        int sockfd = removeAndResetChannel(); //之后会用TcpConnection内的Channel进行读写
        setState(kConnected);
        if(connect_){
            newConnectionCallback_(sockfd);
        }
        else{
            ::close(sockfd);
        }
    }
}

void Connector::handleError(){
    std::cerr << "Connector::handleError state= " << state_;
    if(state_ == kConnecting){
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::restart(){
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}