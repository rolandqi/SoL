/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 14:16:07
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 15:40:24
 */

#ifndef NET_EVENTLOOP_H
#define NET_EVENTLOOP_H


#include "../base/Thread.h"
#include "../base/Mutex.h"
#include "Epoll.h"
#include "Channel.h"

#include <vector>
#include <iostream>

using namespace std;

class EventLoop {
public:
    typedef std::function<void ()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void runInLoop(Functor&& cb);
    void queueInLoop(Functor&& cb);
    inline void removeFromPoller(shared_ptr<Channel> channel)
    {
        poller->epoll_del(channel);
    }
    inline void updatePoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        poller->epoll_mod(channel, timeout);
    }
    inline void addToPoller(shared_ptr<Channel> channel, int timeout = 0)
    {
        poller->epoll_add(channel, timeout);
    }

private:
    shared_ptr<Epoll> poller;
    int wakeupFd_;
    bool callingPendingFunctors_;
    vector<Functor> pendingFunctors_;
    shared_ptr<Channel> pwakeupChannel_;
    base::MutexLock mutex;
    vector<std::shared_ptr<Channel>> activeChannels;
    bool looping;
    void wakeup();
    void handleRead();
    void doPendingFunctors();
    void handleConn();
};

#endif
