//
// Created by Yuchen Qin on 2021/9/18.

#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"
#include <iostream>
#include <memory>
#include <unistd.h>

__thread EventLoop* loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

EventLoop::EventLoop(): looping(false), poller_(new Poller(this)),
    threadId_(std::this_thread::get_id()), quit_(false) {
   if(loopInThisThread != nullptr) {
       abortNotInLoopThread();
   }
   else{
       loopInThisThread = this;
       auto res = pipe(wakeupPipe);
       if(res == -1){
           std::cerr << "Error when piping" << std::endl;
           exit(1);
       }
       wakeupChannel_.reset(new Channel(this, wakeupPipe[0]));
       wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
       wakeupChannel_->enableReading();
   }
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();

    looping = false;
    quit_ = true;
    poller_.reset(nullptr);
    loopInThisThread = nullptr;

}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
    return loopInThisThread;
}

void EventLoop::loop() {
    assert(!looping);
    assertInLoopThread();
    looping = true;
    quit_ = false;

    while(!quit_){
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);
        for(auto iter = activeChannels_.cbegin(); iter != activeChannels_.cend(); iter++){
            (*iter)->handleEvent();
        }
        doPendingFunctors();
    }

    looping = false;
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::runInLoop(const std::function<void()> &cb) {
    if(isInLoopThread()){
        cb();
    }
    else{
        {
            std::lock_guard<std::mutex> lk_(mtx_);
            pendingFunctors_.push_back(cb);
        }
        if(!isInLoopThread() || callingPendingFunctors_)
            wakeup();
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<std::function<void()>> callbacks;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lk_(mtx_);
        callbacks.swap(pendingFunctors_);
    }

    for(const std::function<void()> & cb: callbacks){
        cb();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::wakeup() {
    char buffer[10];
    buffer[0] = 'A';
    auto write_bytes = write(wakeupPipe[1], buffer, 1);
}

void EventLoop::handleRead() {
    char buffer[10];
    auto read_bytes = read(wakeupPipe[0], buffer, 1);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}

//

