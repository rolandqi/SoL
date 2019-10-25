/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:23:36
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 15:18:01
 */
#include "Thread.h"

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

int Thread::numCreated_;

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
    std::cout << "Failed in pthread_create";
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}