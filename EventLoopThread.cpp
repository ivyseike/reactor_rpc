//
// Created by Yuchen Qin on 2021/9/28.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    : loop_(nullptr), exiting_(false), callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if(loop_ != nullptr){ //有可能出现竞态，loop_同时在析构
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    thread_ = std::thread(std::bind(&EventLoopThread::threadFunc, this));
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> ul_(mtx_);
        cond_.wait(ul_, [this](){ return loop_ != nullptr;} );
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if(callback_){
        callback_(&loop);
    }
    {
        std::lock_guard<std::mutex> lk_(mtx_);
        loop_ = &loop;
    }
    cond_.notify_all();
    loop.loop();


    std::lock_guard<std::mutex> lk_(mtx_);
    loop_ = nullptr;
}