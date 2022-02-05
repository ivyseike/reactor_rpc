#ifndef TESTP_POLLPOLLER_H
#define TESTP_POLLPOLLER_H

#include "Poller.h"
#include <poll.h>

class PollPoller: public Poller{
public:
    using Poller::Poller;

    ~PollPoller() = default;

    void poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel);

private:
    using PollFdList = std::vector<pollfd>;
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const override;

    PollFdList pollfds_; //pollfd是poll使用的结构体
};



#endif