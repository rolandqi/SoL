/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 18:19:04
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-16 20:01:14
 */
#ifndef BASE_CONDITION_H_
#define BASE_CONDITION_H_

#include "Mutex.h"
#include <pthread.h>

class Condition
{
public:
    explicit Condition(MutexLock& mutex)
        : mutex_(mutex)
    {
        MCHECK(pthread_cond_init(&pcond_, NULL));
    }

    ~Condition()
    {
      MCHECK(pthread_cond_destroy(&pcond_));
    }

    void wait()
    {
      MCHECK(pthread_cond_wait(&pcond_, mutex_.get()));
    }

    // returns true if time out, false otherwise.
    bool waitForSeconds(double seconds);

    void notify()
    {
      MCHECK(pthread_cond_signal(&pcond_));
    }

    void notifyAll()
    {
      MCHECK(pthread_cond_broadcast(&pcond_));
    }

private:

     MutexLock& mutex_;
     pthread_cond_t pcond_;
};

#endif /* BASE_CONDITION_H_ */
