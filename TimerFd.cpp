//
// Created by Yuchen Qin on 2021/9/26.
//

#include "TimerFd.h"
#include <unistd.h>
#include <iostream>
#include <poll.h>
#include <thread>

TimerFd::TimerFd(): isTiming_(false) {
    if(pipe(pipeFd_) == -1){
        std::cerr << "Pipe failed" << std::endl;
        exit(1);
    }
}

TimerFd::~TimerFd() {
    close(pipeFd_[1]);
    close(pipeFd_[0]);
}

void TimerFd::setTime(const std::chrono::milliseconds &timeout) {
    if(!isTiming_){
        auto timeoutMs = static_cast<int>(timeout.count());
        isTiming_ = true;
        std::thread timer([this, timeoutMs] {
            poll(nullptr, 0, timeoutMs);
            write(pipeFd_[1], " ", 1);
            isTiming_ = false;
        });

        timer.detach();
        std::cout << "Timing!" << std::endl;
    }
    else
        std::cerr << "Timer is running..." << std::endl;
}
