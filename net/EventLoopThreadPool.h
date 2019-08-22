/*
 * EventLoopThreadPool.h
 *
 *  Created on: Aug 21, 2019
 *      Author: kaiqi
 */

#ifndef NET_EVENTLOOPTHREADPOOL_H_
#define NET_EVENTLOOPTHREADPOOL_H_

#include "base/noncopyable.h"
#include "EventLoopThread.h"
#include "base/Logging.h"
#include <memory>
#include <vector>

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);

    ~EventLoopThreadPool()
    {
        LOG << "~EventLoopThreadPool()";
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
