#ifndef CACHEBUFFER_H
#define CACHEBUFFER_H

#include <vector>
#include <mutex>
#include <condition_variable>

#include <assert.h>
// Author: patientCat
// 经过修改的有界队列

// 其大致思想是这样的。这个队列可以存放capacity个frame，每个frame大小是frameSize。比较常见的循环队列实现。
// frame的概念主要取自通信。
template <typename T>
class CacheBuffer {
public:
    CacheBuffer(int capacity, int frameSize)
        : frameSize_(frameSize)
        , size_(0)
        , queue_(capacity) {
        for(int i = 0; i < queue_.size(); i++) {
            queue_[i] = new T[frameSize_];
        }
    }
    ~CacheBuffer() {
        for(int i = 0; i < queue_.size(); i++) {
            delete []queue_[i];
        }
    }

    bool empty() {
        return (size_ == 0);
    }
    bool full() {
        return (size_ == queue_.size());
    }
    void put(const T *start, int len) {
        assert(len == size) ;
        std::unique_lock<std::mutex> lk(mutex_);
        isNotFull_.wait(lk, [&] {	return !full();} );
        // printf("size = %d, capacity_ = %d\n", size_, capacity_);
        for (int i = 0; i < frameSize_; i++) {
            queue_[rear_][i] = start[i];
        }
        rear_ = (rear_ + 1) % queue_.size();
        size_++;
        lk.unlock();

        isNotEmpty_.notify_one();
    }

    void get(T *const start, int len) {
        assert(size == len);
        std::unique_lock<std::mutex> lk(mutex_);
        isNotEmpty_.wait(lk, [&] {	return !empty(); });
        // printf("before get size = %d, after size = %d\n", size_, size_ - len);
        for(int i = 0; i < frameSize_; i++) {
            start[i] = queue_[head_][i];
        }
        head_ = (head_ + 1) % queue_.size();
        size_--;
        lk.unlock();
        isNotFull_.notify_one();
    }

private:
    int frameSize_;
    int size_{0};
    int head_{0};
    int rear_{0};
    std::vector<T *> queue_;
    std::mutex mutex_;
    std::condition_variable isNotFull_;
    std::condition_variable isNotEmpty_;
};

#endif // CACHEBUFFER_H
