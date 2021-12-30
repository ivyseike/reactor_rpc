//
// Created by Yuchen Qin on 2021/10/13.
//

#ifndef TESTP_EVENTLOOPTHREADPOOL_H
#define TESTP_EVENTLOOPTHREADPOOL_H

#include <functional>
#include <memory>
#include <vector>


class EventLoop;
class EventLoopThread;

class EventLoopThreadPool{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop);
    EventLoopThreadPool(const EventLoopThreadPool&) = delete;
    EventLoopThreadPool& operator=(const EventLoopThreadPool&) = delete;
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();
    EventLoop* getLoopAtIndex(size_t index);
    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }

private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;



};

#endif //TESTP_EVENTLOOPTHREADPOOL_H
