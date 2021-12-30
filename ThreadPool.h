//
// Created by Yuchen Qin on 2021/9/16.
//

#ifndef TESTP_THREADPOOL_H
#define TESTP_THREADPOOL_H

#include "ThreadSafeQueue.h"
#include "ThreadJoiner.h"
#include <atomic>
#include <vector>
#include <thread>
#include <future>
#include <memory>

class ThreadPool{
public:
    explicit ThreadPool(int size = std::thread::hardware_concurrency()): running(true), tj(threads) {
        for(auto i = 0; i < size; i++){
            threads.emplace_back(std::thread(&ThreadPool::worker_thread, this));
        }
    }
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ~ThreadPool(){
        running = false;
        cond_.notify_all();
    }
    template<typename FunctionType, typename... Args> //函数类型和具体参数
    auto submit(FunctionType &&f, Args &&...args) -> std::future<decltype(f(args...))>{
        std::function<decltype(f(args...))()> func = std::bind(std::forward<FunctionType>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        auto wrapper_func = [task_ptr] () { (*task_ptr)(); };
        {
           std::lock_guard<std::mutex> lk_(m_);
           tasks.push(wrapper_func);
           cond_.notify_one();
        }

        return task_ptr->get_future();
    }

private:
    void worker_thread(){
        while(running) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> ul_(m_);
                cond_.wait(ul_, [this] { return !running || !tasks.empty(); });

                if(!running)
                    return;
                task = std::move(tasks.front());
                tasks.pop();
            }
            ;
        }
    }
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> threads;
    std::atomic_bool running;
    std::mutex m_;
    std::condition_variable cond_;
    ThreadJoiner tj;
};



#endif //TESTP_THREADPOOL_H
