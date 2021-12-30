//
// Created by Yuchen Qin on 2021/9/28.
//

#ifndef TESTP_EVENTLOOPTHREAD_H
#define TESTP_EVENTLOOPTHREAD_H

#include <thread>

class EventLoop;

class EventLoopThread{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread& operator=(const EventLoopThread&) = delete;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;

};



#endif //TESTP_EVENTLOOPTHREAD_H
