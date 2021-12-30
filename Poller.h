//
// Created by Yuchen Qin on 2021/9/19.
//

#ifndef TESTP_POLLER_H
#define TESTP_POLLER_H

#include <vector>
#include <unordered_map>
#include "EventLoop.h"
#include "Channel.h"
#include <poll.h>

class Poller{
    //Poller doesn't own Channel
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop); //只属于一个Eventloop
    ~Poller() = default;
    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    void poll(int timeoutMs, ChannelList *activeChannels);
    void updateChannel(Channel* channel);
    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }
    bool hasChannel(Channel* channel){ return channels_.find(channel->fd()) != channels_.end(); }
    void removeChannel(Channel* channel);

private:
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    using PollFdList = std::vector<pollfd>;
    using ChannelMap = std::unordered_map<int, Channel*>;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

#endif //TESTP_POLLER_H
