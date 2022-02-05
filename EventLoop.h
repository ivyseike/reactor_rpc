//
// Created by Yuchen Qin on 2021/9/18.
//

#ifndef TESTP_EVENTLOOP_H
#define TESTP_EVENTLOOP_H

#include <thread>
#include <iostream>
#include <vector>
#include <mutex>

class Channel;
class Poller;

class EventLoop{
public:
    EventLoop(int poller = 0);
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
    void loop();
    void assertInLoopThread(){
        if(!isInLoopThread()) abortNotInLoopThread();
    }
    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }
    static EventLoop* getEventLoopOfCurrentThread();
    void quit(){
        quit_ = true;
        if(!isInLoopThread())
            wakeup();
    }
    void updateChannel(Channel *channel);
    void runInLoop(const std::function<void()> &cb);
    void wakeup();

    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

private:
    using ChannelList = std::vector<Channel*>;
    void abortNotInLoopThread(){
        std::cerr << "Something went wrong" << std::endl;
        exit(1);
    }
    void doPendingFunctors();
    void handleRead();

    std::atomic_bool looping;
    std::atomic_bool quit_;
    std::thread::id threadId_;
    std::unique_ptr<Poller> poller_;
    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_;
    int wakeupPipe[2]; //to wake up from polling
    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<std::function<void()>> pendingFunctors_;
    std::mutex mtx_;
};

#endif //TESTP_EVENTLOOP_H
