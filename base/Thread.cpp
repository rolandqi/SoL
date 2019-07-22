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


namespace detail
{
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
        *tid_ = muduo::CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;

        CurrentThread::t_threadName =
                name_.empty() ? "myThread" : name_.c_str();
        ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);  // 给线程命名
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

    typedef ThreadL::ThreadFunc ThreadFunc;
    ThreadFunc func;
    string name_;
    pid_t* tid_;
    CountDownLatch* latch_;
};

void startThread(void *obj)
{
    ThreadData* data = static_cats<ThreadData*>(obj);
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
        t_tidstringLength = snprintf(st_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}






