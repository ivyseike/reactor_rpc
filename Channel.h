//
// Created by Yuchen Qin on 2021/9/18.
//

#ifndef TESTP_CHANNEL_H
#define TESTP_CHANNEL_H

//每个Channel从始至终只属于一个Eventloop，只负责一个fd，但不拥有fd

#include <memory>
#include <functional>
class EventLoop; //forward declaration

class Channel{
public:
    using EventCallback = std::function<void()> ; //回调

    Channel(EventLoop *loopPtr, int fd);
    ~Channel();

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    void handleEvent();
    void setReadCallback(const EventCallback &cb) { readCallback_ = cb;} //用户使用
    void setWriteCallback(const EventCallback &cb) { writeCallback_ = cb;}
    void setErrorCallback(const EventCallback &cb) { errorCallback_ = cb;}
    void setCloseCallback(const EventCallback &cb) { closeCallabck_ = cb;}

    int fd() const { return fd_;}
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; };

    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }

    void disableReading() { events_ &= ~kReadEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    //for poller
    int index() { return index_;}
    void set_index(int idx) { index_ = idx; }
    EventLoop* ownerLoop() { return loopPtr_; }

    void remove();


private:
    void update();
    static const int kNoneEvent; //POSIX的常量
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loopPtr_;
    const int fd_;
    int events_; //用户关心的IO事件，由用户设置
    int revents_; //目前活动的事件，由EventLoop/Poller设置
    int index_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback  closeCallabck_;

    bool handlingEvent_;
    bool addedToLoop_;

};


#endif //TESTP_CHANNEL_H

