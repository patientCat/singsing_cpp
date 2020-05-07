#include "countdownlatch.h"

CountDownLatch::CountDownLatch(int count)
    : mutex_()
    , condition_()
    , count_(count)
{}


void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> lk(mutex_);
    condition_.wait(lk, [&](){
        return count_ == 0;
    });
}

void CountDownLatch::countDown()
{
    std::lock_guard<std::mutex> lk(mutex_);
    --count_;
    if(0==count_)
    condition_.notify_one();
}
int CountDownLatch::getCount() const
{
    std::lock_guard<std::mutex> lk(mutex_);
    return count_;
}
