/*
 * Thread.h
 *
 *  Created on: Jul 22, 2019
 *      Author: kaiqi
 */

#ifndef BASE_THREAD_H_
#define BASE_THREAD_H_

#include "base/CountDownLatch.h"
#include "base/noncopyable.h"
#include "base/Atomic.h"



#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>
#include <string>

class Thread : noncopyable
{
public:
    typedef std::funtion<void()> ThreadFunc;

    explicit Thread(ThreadFunc, const string& name = string());
    ~Thread();

    void start();
    int join();
    bool started() const {return started_;}
    pid_t tid() const {return tid_;}
    const string & name() const {return name_;}
    static int numCreated() {return numCreated_.get()}  // TODO 这个函数能写成const吗？

private:
    void setDefaultName();
    bool started_;
    bool joined_;
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_t;
    string name_;
    CountDownLatch latch_;
    static AtomicInt32 numCreated_;
};


#endif /* BASE_THREAD_H_ */
