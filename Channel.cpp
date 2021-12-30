//
// Created by Yuchen Qin on 2021/9/18.

#include "Channel.h"
#include "EventLoop.h"
#include <poll.h>
#include <unistd.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI; //POLLPRI：高优先级数据可读
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loopPtr, int fd): handlingEvent_(false), addedToLoop_(false),
    loopPtr_(loopPtr), fd_(fd), events_(0), revents_(0), index_(-1) {}


void Channel::handleEvent() {//由EventLoop::loop()调用
    handlingEvent_ = true;
    if(revents_ & POLLNVAL){ //描述字不是一个打开的文件
        std::cerr << "Error when Channel handling event" << std::endl;
        exit(1);
    }

    if((revents_ & POLLHUP) && !(revents_ & POLLIN)){
        if(closeCallabck_) closeCallabck_();
    }

    if(revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }

    if(revents_ & (POLLIN | POLLPRI | POLLHUP)){
        if(readCallback_) readCallback_(); //readCallback将持有相应fd，所以不需要传递数据
    }

    if(revents_ & POLLOUT){
        if(writeCallback_) writeCallback_();
    }
    handlingEvent_ = false;
}

void Channel::update() {
    loopPtr_->updateChannel(this);
    addedToLoop_ = true;
}

void Channel::remove(){
    assert(isNoneEvent());
    addedToLoop_ = false;
    loopPtr_->removeChannel(this);
}

Channel::~Channel(){
    //assert(!handlingEvent_);
    assert(!addedToLoop_);
}




