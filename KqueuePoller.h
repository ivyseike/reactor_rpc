#ifndef TESTP_KQUEUEPOLLER_H
#define TESTP_KQUEUEPOLLER_H

#include "Poller.h"
#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>

class KqueuePoller: public Poller{
public:
    KqueuePoller(EventLoop* loop): Poller(loop){
        kqfd = kqueue();
    }

    ~KqueuePoller(){
        close(kqfd);
    }

    void poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
    
private:
    virtual void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    using KqueueFdList = std::vector<struct kevent>;
    int kqfd;

    KqueueFdList fdlist_;


};

#endif
