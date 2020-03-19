/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-17 14:03:48
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 19:32:04
 */
#ifndef NET_EVENTLOOPTHREADPOOL_H_
#define NET_EVENTLOOPTHREADPOOL_H_

#include "EventLoopThread.h"
#include <vector>

class EventLoopThreadPool
{
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);

    ~EventLoopThreadPool()
    {
        LOG_INFO << "~EventLoopThreadPool()";
    }
    void start();

    EventLoop *getNextLoop();

private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};



#endif /* NET_EVENTLOOPTHREADPOOL_H_ */
