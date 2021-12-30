//
// Created by Yuchen Qin on 2021/10/13.
//

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop):
                    baseLoop_(baseLoop), started_(false), numThreads_(0), next_(0) {}


void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();

    for(auto i = 0; i < numThreads_; i++){
        std::unique_ptr<EventLoopThread> up(new EventLoopThread(cb));
        loops_.push_back(up->startLoop());
        threads_.push_back(std::move(up));
    }

    if(numThreads_ == 0 && cb){
        cb(baseLoop_);
    }

    started_ = true;
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if(!loops_.empty()){
        loop = loops_[next_++];
        if(next_ >= loops_.size()) next_ = 0;
    }

    return loop;
}

EventLoop *EventLoopThreadPool::getLoopAtIndex(size_t index) {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if(!loops_.empty()){
        loop = loops_[index % loops_.size()];
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();
    assert(started_);

    if(!loops_.empty())
        return loops_;
    else
        return std::vector<EventLoop*>{ baseLoop_ };
}


