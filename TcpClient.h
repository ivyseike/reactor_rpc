//
// Created by Yuchen Qin on 2021/11/1.
//

#ifndef TESTP_TCPCLIENT_H
#define TESTP_TCPCLIENT_H

#include "TcpConnection.h"
#include "InetAddress.h"
#include <mutex>

class Connector;


class TcpClient {
public:
    using ConnectorPtr = std::shared_ptr<Connector>;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)> ;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, const char* buffer, int len)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& nameArg);

    //~TcpClient();

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const{
        std::lock_guard<std::mutex> lk(mutex_);
        return connection_;
    }

    EventLoop* getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

    const std::string& name() const {return name_; }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }



private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    const std::string name_;
    ConnectorPtr connector_;
    TcpConnectionPtr  connection_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;

    std::atomic_bool retry_;
    std::atomic_bool connect_;

    int nextConnId_;
    mutable std::mutex mutex_;

    InetAddress serverAddr_;



};


#endif //TESTP_TCPCLIENT_H
