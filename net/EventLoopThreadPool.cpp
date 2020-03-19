/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-17 14:03:53
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 15:20:22
 */

#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, int numThreads)
:   baseLoop_(baseLoop),
    started_(false),
    numThreads_(numThreads),
    next_(0)
{
    if (numThreads_ <= 0)
    {
        LOG_INFO << "numThreads_ <= 0";
        abort();
    }
}

void EventLoopThreadPool::start()
{
    started_ = true;
    for (int i = 0; i < numThreads_; ++i)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    assert(started_);
    EventLoop *loop = baseLoop_;
    if (!loops_.empty())
    {
        loop = loops_[next_];
        next_ = (next_ + 1) % numThreads_;
    }
    return loop;
}

