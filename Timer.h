//
// Created by Yuchen Qin on 2021/9/27.
//

#ifndef TESTP_TIMER_H
#define TESTP_TIMER_H

#include <atomic>
#include <chrono>
#include <functional>

class Timer{
public:
    using TimePoint = std::chrono::system_clock::time_point ;
    using SecondDuration = std::chrono::duration<double>;
    using TimerCallback = std::function<void()>;

    Timer() = delete;
    Timer(TimerCallback &callback, TimePoint exp, SecondDuration interval): callback_(callback), interval_(interval),
                                expiration_(exp), sequence_(created_timer++), repeat_(interval > 0.0) { }
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void run() const { callback_(); }

    TimePoint expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    std::atomic<int64_t> sequence() const { return sequence_; }

    void restart(TimePoint now)
    {
        if (repeat_)
            expiration_ = now + interval_;
        else
            expiration_ = std::chrono::system_clock::now() - SecondDuration(1);
    }


    static int64_t timerCreated() { return created_timer; }

private:
    static std::atomic<int64_t> created_timer = 0;
    const int64_t sequence_;
    const TimerCallback callback_;
    const SecondDuration interval_;
    const bool repeat_;
    std::chrono::system_clock::time_point expiration_;
};

#endif //TESTP_TIMER_H
