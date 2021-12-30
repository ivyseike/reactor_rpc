//
// Created by Yuchen Qin on 2021/9/26.
//

#ifndef TESTP_TIMERFD_H
#define TESTP_TIMERFD_H

#include <chrono>
#include <atomic>

class TimerFd{
public:
    TimerFd();
    ~TimerFd();
    TimerFd(const TimerFd&) = delete;
    TimerFd& operator=(const TimerFd&) = delete;

    int fd() const { return pipeFd_[0]; } //read
    void setTime(const std::chrono::milliseconds &timeout);

private:
    int pipeFd_[2];
    std::atomic<bool> isTiming_;
};
#endif //TESTP_TIMERFD_H
