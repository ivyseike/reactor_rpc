//
// Created by Yuchen Qin on 2021/10/13.
//

#ifndef TESTP_TCPCONNECTION_H
#define TESTP_TCPCONNECTION_H

#include "InetAddress.h"
#include <memory>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

class TcpConnection;

void defaultConnectionCallback(const std::shared_ptr<TcpConnection>&);
void defaultMessageCallback(const std::shared_ptr<TcpConnection>&, const char*, int);

class TcpConnection: public std::enable_shared_from_this<TcpConnection>{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)> ;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, const char* buffer, int len)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

    friend void defaultConnectionCallback(const TcpConnectionPtr&);
    friend void defaultMessageCallback(const TcpConnectionPtr&, const char*, int);

    TcpConnection(EventLoop* loop, int sockfd, const InetAddress& localAddr,
                  const InetAddress& peerAddr, std::string name);

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    bool connected() const { return state_ == kConnected; }

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; } // intenal use only

    void connectEstablished(); // called when TcpServer accepts a new connection
    void connectDestroyed(); // called when TcpServer has removed me from its map

    std::string name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }
    InetAddress peerAddress() const { return peerAddr_; }
    InetAddress localAddress() const { return localAddr_; }

    void send(const std::string& message);
    void send(const char* message, int len);
    void sendInLoop(const std::string& message);
    void sendInLoopChar(const char* message, int len);
    void shutdown();
    void shutdownInLoop();

    void setTcpNoDelay(bool on);
    void setTcpKeepAlive(bool on);


private:
    enum StateE {kConnecting, kConnected, kDisconnecting, kDisconnected, };
    StateE state_;
    void setState(StateE s) { state_ = s; }

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop* loop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress localAddr_; //自己的地址
    InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback  closeCallback_;

    std::string name_;

    std::string outputBuffer_;

};

#endif //TESTP_TCPCONNECTION_H
