/*
 * CurrentThread.h
 *
 *  Created on: Jul 22, 2019
 *      Author: kaiqi
 */

#ifndef BASE_CURRENTTHREAD_H_
#define BASE_CURRENTTHREAD_H_

namespace CurrentThread
{
    // internal
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "unknown";
    void cacheTid();
    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char* tidString() // for logging
    {
        return t_tidString;
    }

    inline int tidStringLength() // for logging
    {
        return t_tidStringLength;
    }

    inline const char* name()
    {
        return t_threadName;
    }
    bool isMainThread();
    void sleepingUsec(int64_t usec);
    std::string stackTrace(bool demangle);
}



#endif /* BASE_CURRENTTHREAD_H_ */
