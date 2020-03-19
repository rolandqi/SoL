/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 14:37:25
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 11:31:28
 */
#include "EventLoop.h"
#include <sys/eventfd.h>

using namespace base;

// epoll调用的超时事件，默认10s
const int kPollTimeMs = 10000;

int createEventfd()
{
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG_INFO << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

void EventLoop::loop()
{
    assert(!looping);
    looping = true;
    while (true)
    {
        activeChannels.clear();
        poller->poll(kPollTimeMs, &activeChannels);
        for (auto iter = activeChannels.begin(); iter != activeChannels.end(); iter++) {
            (*iter)->handleEvents();
        }
        doPendingFunctors();
    }
    LOG_INFO <<"end looping in "<< this;
    looping = false;  // TODO: quit looping mechnism.
}

EventLoop::EventLoop()
  : poller(new Epoll()),
    wakeupFd_(createEventfd()),
    looping(false),
    callingPendingFunctors_(false),
    pwakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_INFO <<"Event loop created in " << this; 
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
    pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
    pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
    poller->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_INFO << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
    //pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::handleConn()
{
    //poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET | EPOLLONESHOT), 0);
    updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() 
{
    LOG_INFO <<"Event loop deleted in " << this;
    close(wakeupFd_);
    removeFromPoller(pwakeupChannel_);
}

void EventLoop::runInLoop(Functor&& cb)
{
    cb();
}

void EventLoop::queueInLoop(Functor&& cb)
{
    {
       base::MutexLockGuard lock(mutex);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!callingPendingFunctors_)
        wakeup();
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        base::MutexLockGuard lock(mutex);
        functors.swap(pendingFunctors_);  // clean pending functors
    }

    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();
    callingPendingFunctors_ = false;
}


void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
    if (n != sizeof one)
    {
        LOG_INFO << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}