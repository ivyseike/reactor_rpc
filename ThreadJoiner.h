//
// Created by Yuchen Qin on 2021/9/16.
//

#ifndef TESTP_THREADJOINER_H
#define TESTP_THREADJOINER_H

#include <thread>
#include <vector>
#include <memory>

class ThreadJoiner{
public:
    explicit ThreadJoiner(std::vector<std::thread> &ptr): threadsPtr(ptr) {}
    ~ThreadJoiner(){
        for (unsigned int i = 0; i < threadsPtr.size(); i++)
            if (threadsPtr[i].joinable())
                threadsPtr[i].join();
    }

private:
    std::vector<std::thread> &threadsPtr;
};

#endif //TESTP_THREADJOINER_H
