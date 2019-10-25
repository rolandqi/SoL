/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 18:15:31
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 10:36:25
 */
#ifndef BASE_MUTEX_H_
#define BASE_MUTEX_H_

#include <assert.h>
#include <pthread.h>
#include <stddef.h>

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

class MutexLock
{
public:
    MutexLock()
    {
        MCHECK(pthread_mutex_init(&mutex_, NULL));
    }

    ~MutexLock()
    {
        MCHECK(pthread_mutex_destroy(&mutex_));
    }

    void lock()
    {
        MCHECK(pthread_mutex_lock(&mutex_));
    }

    void unlock()
    {
        MCHECK(pthread_mutex_unlock(&mutex_));
    }

    pthread_mutex_t *get()
    {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
    friend struct Condition;
};

struct MutexLockGuard
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
