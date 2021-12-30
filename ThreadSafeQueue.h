//
// Created by Yuchen Qin on 2021/9/16.
//

#ifndef TESTP_THREADSAFEQUEUE_H
#define TESTP_THREADSAFEQUEUE_H

#include <thread>
#include <queue>


template <typename T>
class ThreadSafeQueue{
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue &) = delete; //不能拷贝构造或者赋值
    ThreadSafeQueue& operator= (const ThreadSafeQueue &) = delete;
    void enqueue(const T& );
    bool nonBlockingDequeue(T& );
    void blockingDequeue(T& );
    bool empty() const;
    int size() const;
private:
    mutable std::mutex m_;
    std::queue<T> q_;
    std::condition_variable cond_;
};
template <typename T>
void ThreadSafeQueue<T>::enqueue(const T& val){
    {
        std::lock_guard<std::mutex> lk_(m_);
        q_.push(val);
    }
    cond_.notify_one();

}

template <typename T>
bool ThreadSafeQueue<T>::nonBlockingDequeue(T & val){
    std::lock_guard<std::mutex> lk_(m_);
    if(q_.empty())
        return false;
    val = q_.front();
    q_.pop();
    return true;
}

template <typename T>
void ThreadSafeQueue<T>::blockingDequeue(T & val){
    std::unique_lock<std::mutex> ul_(m_);
    cond_.wait(ul_, [this] {return !q_.empty();}); // needs capture
    val = std::move(q_.front());
    q_.pop();
}

template <typename T>
bool ThreadSafeQueue<T>::empty() const{
    std::lock_guard<std::mutex> lk_(m_);
    return q_.empty();
}

template <typename T>
int ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lk_(m_);
    return q_.size();
}

#endif //TESTP_THREADSAFEQUEUE_H
