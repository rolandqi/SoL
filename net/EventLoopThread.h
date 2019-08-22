/*
 * EventLoopThread.h
 *
 *  Created on: Aug 21, 2019
 *      Author: kaiqi
 */

#ifndef NET_EVENTLOOPTHREAD_H_
#define NET_EVENTLOOPTHREAD_H_
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"
#include "EventLoop.h"

class EventLoopThread :noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
};




#endif /* NET_EVENTLOOPTHREAD_H_ */
