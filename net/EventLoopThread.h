/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-17 14:19:12
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 17:38:31
 */
#ifndef NET_EVENTLOOPTHREAD_H_
#define NET_EVENTLOOPTHREAD_H_

#include "../base/Condition.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"
#include "EventLoop.h"

class EventLoopThread
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop *loop_;
    bool exiting_;
    base::Thread thread_;
    base::MutexLock mutex_;
    base::Condition cond_;
};




#endif /* NET_EVENTLOOPTHREAD_H_ */