#ifndef CIRCULARBUFFERTHREADSAFE_H
#define CIRCULARBUFFERTHREADSAFE_H

#include <boost/noncopyable.hpp>

#include <thread>
#include <mutex>

#include <stdio.h>

// Author: patientCat

// 线程安全的循环队列
// 本api名字大多模仿自muduo::net::buffer，如果有幸阅读过将很容易理解
template<typename T>
class CirularBufferThreadSafe
    : public boost::noncopyable {
public:
    explicit CirularBufferThreadSafe(int n)
        : size_(0)
        , capacity_(n)
        , head_(0)
        , rear_(0)
        , mutex_()
        , isNotFull_()
        , isNotEmpty_() {
        data_ = new T[capacity_ + 1];
    }
    ~CirularBufferThreadSafe() {
        delete[] data_;
    }

	/*
	*	从start开始到start+len位置的数据置入队列。
	*/
    void put(const T *start, int len) {
        std::unique_lock<std::mutex> lk(mutex_);
        isNotFull_.wait(lk, [&] {	return (size_ + len) <= capacity_; });
        // printf("size = %d, capacity_ = %d\n", size_, capacity_);
        for (int i = 0; i < len; i++) {
            data_[rear_] = start[i];
            rear_ = (rear_ + 1) % capacity_;
        }
        // printf("before put size = %d, after size = %d\n", size_, size_ + len);
        size_ += len;
        lk.unlock();

        isNotEmpty_.notify_one();
    }
    /*
	*	单纯移动head_标记。配合peek使用。
	*/
    void retrieve(int len) {
        std::unique_lock<std::mutex> lk(mutex_);
        isNotEmpty_.wait(lk, [&] {	return (size_ - len) >= 0; });
        head_ = (head_ + len) % capacity_;
        size_ -= len;

        lk.unlock();
        isNotFull_.notify_one();
    }
    /*
	*	查看元素，但是不移动head_标记。配合retrieve使用。
	*/
    void peek(T *const start, int len) {
        std::unique_lock<std::mutex> lk(mutex_);
        isNotEmpty_.wait(lk, [&] {	return (size_ - len) >= 0; });
        for(int i = 0; i < len; i++) {
            start[i] = data_[(head_ + i) % capacity_];
        }
        lk.unlock();
    }
	/*
	*	从队列中取出len个元素置于到start到start+len位置。
	*/
    void get(T *const start, int len) {
        std::unique_lock<std::mutex> lk(mutex_);
        isNotEmpty_.wait(lk, [&] {	return (size_ - len) >= 0; });
        // printf("before get size = %d, after size = %d\n", size_, size_ - len);
        for(int i = 0; i < len; i++) {
            start[i] = data_[head_];
            head_ = (head_ + 1) % capacity_;
        }
        size_ -= len;
        lk.unlock();

        isNotFull_.notify_one();
    }
    int size() const {
        std::lock_guard<std::mutex> guard(mutex_);
        return size_;
    }
    int capacity() const {
		std::lock_guard<std::mutex> guard(mutex_);
        return capacity_;
    }
private:
    int size_;
    int capacity_;
    int head_;
    int rear_;
    T *data_;

    std::mutex mutex_;
    std::condition_variable isNotFull_;
    std::condition_variable isNotEmpty_;
};


#endif // CIRCULARBUFFERTHREADSAFE_H
