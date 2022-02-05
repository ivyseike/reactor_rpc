//
// Created by Yuchen Qin on 2021/9/19.
//

#include "PollPoller.h"

void PollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    if(numEvents > 0)
        fillActiveChannels(numEvents, activeChannels);
}
void PollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    //tranverse pollfds_
    for(auto iter = pollfds_.cbegin(); numEvents > 0 && iter != pollfds_.cend();
        iter++){
        if(iter->revents > 0){
            --numEvents;
            auto iter2 = channels_.find(iter->fd);
            assert(iter2 != channels_.end());
            Channel * channelPtr = iter2->second;
            assert(channelPtr->fd() == iter->fd);
            channelPtr->set_revents(iter->revents);
            activeChannels->push_back(channelPtr);
        }
    }
}

void PollPoller::updateChannel(Channel *channel) {
    if(channel->index() < 0){ //index in pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        channel->set_index(static_cast<int>(pollfds_.size())-1);
        channels_[pfd.fd] = channel;
    }
    else{
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0; //restart to listen
        if(channel->isNoneEvent()){
            pfd.fd= -1; //ignore this pollfd
        }

    }
}

void PollPoller::removeChannel(Channel *channel) {
    auto iter = channels_.find(channel->fd());
    assert(iter != channels_.end());
    pollfds_.erase(pollfds_.begin()+iter->second->index());
    channels_.erase(iter);
    channel->set_index(-1);
}





