//
// Created by Yuchen Qin on 2021/10/9.
//

#ifndef TESTP_ACCEPTOR_H
#define TESTP_ACCEPTOR_H

#include "Channel.h"
#include "Socket.h"

class InetAddress;
class EventLoop;

class Acceptor{
public:
    using NewConnectionCallback = std::function<void(int connFd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }

    void listen();

    bool listening() const { return listening_; }

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;
};

#endif //TESTP_ACCEPTOR_H
