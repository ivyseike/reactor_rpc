//
// Created by Yuchen Qin on 2021/9/18.
//

#ifndef TESTP_COUNTDOWNLATCH_H
#define TESTP_COUNTDOWNLATCH_H

#incldue <thread>

class CountdownLatch{
public:
    explicit CountdownLatch(int count): count_(count) {}
    void wait(){
       std::unique_lock<std::mutex> ul_(m_) ;
       cond_.wait(ul_, [this] { return count == 0;});
    }
    void countdown(){
        std::lock_guard<std::mutex> lk_(m_);
        count_--;
        if(count_ == 0){
            cond_.notify_all();
        }
    }
private:
    int count_;
    std::mutex m_;
    std::condition_variable cond_;
};

#endif //TESTP_COUNTDOWNLATCH_H
