#include "KqueuePoller.h"

void KqueuePoller::poll(int timeoutMs, ChannelList *activeChannels){

}
void KqueuePoller::updateChannel(Channel* channel){
    if(channel->index() < 0){ //index in pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct kevent kvt;
        int op = 0;
        if(channel->isReading()) op = EVFILT_READ;
        else if(channel->isWriting()) op = EVFILT_WRITE;
        if(op != 0){
            EV_SET(&kvt, kqfd, op, EV_ADD | EV_ENABLE, 0, 0, NULL);
            kevent(kqfd, &kvt, 1, NULL, 0, NULL);
        }
        fdlist_.push_back(kvt);
        channel->set_index(static_cast<int>(fdlist_.size())-1);
        channels_[channel->fd()] = channel;
    }
    else{
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(fdlist_.size()));
        struct kevent &kvt = fdlist_[idx];
        //assert(pfd.fd == channel->fd() || pfd.fd == -1);
        //pfd.events = static_cast<short>(channel->events());
        //pfd.revents = 0; //restart to listen
        if(!channel->isReading()){

        }
        if(!channel->isWriting()){

        }

    }

}
void KqueuePoller::removeChannel(Channel* channel){

}