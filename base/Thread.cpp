/*
 * Thread.cpp
 *
 *  Created on: Jul 22, 2019
 *      Author: kaiqi
 */
#include "base/Thread.h"
#include "base/CurrentThread.h"

#include <type_traits>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <string>
#include <iostream>

namespace detail
{
pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void afterFork()
{
  CurrentThread::t_cachedTid = 0;
  CurrentThread::t_threadName = "main";
  CurrentThread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;

// 为了在线程中保留name,tid这些数据
struct ThreadData
{
    ThreadData(ThreadFunc func,
            const string& name,
            pid_t* tid,
            CountDownLatch* latch)
    : func_(std::move(func)),  // TODO why移动构造？
      name_(name),
      tid_(tid),
      latch_(latch)
    {}

    void runInThread() {
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;

        CurrentThread::t_threadName =
                name_.empty() ? "myThread" : name_.c_str();
        ::prctl(PR_SET_NAME, CurrentThread::t_threadName);  // 给线程命名
        try {
            func_();
            CurrentThread::t_threadName = "finished";
        } catch (const std::exception& ex) {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        } catch (...) {
            CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n",
                    name_.c_str());
            throw; // rethrow
        }
    }

    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
    CountDownLatch* latch_;
};

void startThread(void *obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return;
}
}  // namespace detail

void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = detail::gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}

bool CurrentThread::isMainThread()
{
    return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
    struct timespec ts = { 0, 0 };
    ts.tv_sec = static_cast<time_t>(usec / 1000000);
    ts.tv_nsec = static_cast<long>(usec % 1000000 * 1000);
    ::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::numCreated_;   // TODO could the code pass compile if we comment this definition?


Thread::Thread(ThreadFunc func, const std::string& n)
    : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(n),
    latch_(1)
{
  setDefaultName();
}

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    pthread_detach(pthreadId_);
  }
}

void Thread::setDefaultName()
{
  int num = numCreated_.incrementAndGet();
  if (name_.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
  {
    started_ = false;
    delete data; // or no delete?
    std::cout << "Failed in pthread_create";
  }
  else
  {
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}




