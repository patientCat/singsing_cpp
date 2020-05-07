#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include <boost/noncopyable.hpp>
#include <mutex>
#include <condition_variable>


// Author: patientCat

// 常用的线程间同步工具
class CountDownLatch : boost::noncopyable
{
public:

explicit CountDownLatch(int count);

void wait();

void countDown();

int getCount() const;

private:

mutable std::mutex mutex_;
std::condition_variable condition_;
int count_;
};

#endif // COUNTDOWNLATCH_H
