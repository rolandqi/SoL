/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:23:36
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 15:18:01
 */
#include "Thread.h"
#include "current_thread.h"
// #include "exception.h"
#include "logging.h"

#include <type_traits>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <iostream>
#include <assert.h>

int base::Thread::numCreated_;
namespace base {

namespace CurrentThread {
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char *t_threadName = "unknown";
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");
}

namespace detail {

pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

void afterFork() {
  base::CurrentThread::t_cachedTid = 0;
  base::CurrentThread::t_threadName = "main";
  CurrentThread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer {
public:
  ThreadNameInitializer() {
    base::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;
}

struct ThreadData
{
public:
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadData(ThreadFunc func, const string& name, pid_t* tid)
    : func_(std::move(func)),
      name_(name),
      tid_(tid)
    {}

    void runInThread() {
        try {
            func_();
        } catch (const std::exception& ex) {
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        } catch (...) {
            fprintf(stderr, "unknown exception caught in Thread %s\n",
                    name_.c_str());
            throw;
        }
    }

    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
};

void* startThread(void *obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread(ThreadFunc func, const std::string& n)
    : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(n)
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
  int num = numCreated_++;
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
  // TODO: move(func_)
  ThreadData* data = new ThreadData(func_, name_, &tid_);
  if (pthread_create(&pthreadId_, NULL, &startThread, data))
  {
    started_ = false;
    delete data; // or no delete?
    LOG_INFO << "Failed in pthread_create";
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}

void CurrentThread::cacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = detail::gettid();
    t_tidStringLength =
        snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread() { return tid() == ::getpid(); }

void CurrentThread::sleepUsec(int64_t usec) {
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec =
      static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, NULL);
}

}