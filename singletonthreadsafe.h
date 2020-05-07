#ifndef SINGLETON_H
#define SINGLETON_H

// 线程安全的单例模式
#include <thread>
#include <mutex>

#include <boost/noncopyable.hpp>

// Author: patientCat

template <typename T>
class SingletonThreadSafe
    : public boost::noncopyable {
public:
    static T *getInstance() {
        std::call_once(flag_, init);
        return instance_;
    }
private:
    static void init() {
        instance_ = new T();
    }
    static T *instance_;
    static std::once_flag flag_;
};

template <typename T>
T *SingletonThreadSafe<T>::instance_ = NULL;
template <typename T>
std::once_flag SingletonThreadSafe<T>::flag_;
#endif // SINGLETON_H
