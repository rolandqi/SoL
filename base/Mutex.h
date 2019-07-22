/*
 * Mutex.h
 *
 *  Created on: Jul 22, 2019
 *      Author: kaiqi
 */

#ifndef BASE_MUTEX_H_
#define BASE_MUTEX_H_

#include "base/CurrentThread.h"
#include "base/noncopyable.h"
#include <assert.h>
#include <pthread.h>

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

class MutexLock : noncopyable
{
public:
    MutexLock()
    {
        MCHECK(pthread_mutex_init(&mutex_, NULL));
    }

    ~MutexLock()
    {
        assert(holder_ == 0);  // 因为mutex和 mutexGuard是分开声明的，所以这里必须用一个assert检查检查是否已经lock住了
        MCHECK(pthread_mutex_destroy(&mutex));
    }

    // must be called when locked, i.e. for assertion
    bool isLockedByThisThread() const
    {
      return holder_ == CurrentThread::tid();
    }

    void lock()
    {
        MCHECK(pthread_mutex_lock(&mutex_));
        assignHolder();
    }

    void unlock()
    {
        unassignHolder();
        MCHECK(pthread_mutex_unlock(&mutex_));
    }

    pthread_mutex_t *get()
    {
        return &mutex;
    }

private:
    pthread_mutex_t mutex;
    friend Condition;
    pid_t holder_;

    void unassignHolder()
    {
      holder_ = 0;
    }

    void assignHolder()
    {
      holder_ = CurrentThread::tid();
    }
};

class MutexLockGuard : noncopyable
{
    explicit MutexLockGuard(MutexLock &_mutex):
        mutex(_mutex)
    {
        mutex.lock();
    }
    ~MutexLockGuard()
    {
        mutex.unlock();
    }
private:
    MutexLock &mutex;  // 声明的就是一个引用
};

#define MutexLockGuard(x) error "Missing mutex guard object name"



#endif /* BASE_MUTEX_H_ */
