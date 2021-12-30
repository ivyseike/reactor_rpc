//
// Created by Yuchen Qin on 2021/10/25.
//

#ifndef TESTP_CONNECTOR_H
#define TESTP_CONNECTOR_H

#include <functional>
#include "InetAddress.h"

class EventLoop;
class Channel;

class Connector: public std::enable_shared_from_this<Connector> {
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }

    void start();
    void restart();
    void stop();

    const InetAddress& serverAddress() const { return serverAddr_; }


private:
    enum States {kDisconnected, kConnecting, kConnected };
    static const int kMaxRetryDelayMs = 30000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) { state_ = s; }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    std::atomic_bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;

};


#endif //TESTP_CONNECTOR_H
