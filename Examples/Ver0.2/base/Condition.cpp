/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 18:18:46
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 17:47:51
 */
#include "Condition.h"

#include <errno.h>
#include <time.h>
#include <sys/types.h>

// returns true if time out, false otherwise.
bool Condition::waitForSeconds(double seconds)
{
  struct timespec abstime;  // condtion lock 使用timespec
  // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
  clock_gettime(CLOCK_REALTIME, &abstime);

  const int64_t kNanoSecondsPerSecond = 1000000000;
  int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

  abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
  abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

  MutexLockGuard lock(mutex_);
  return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.get(), &abstime);
}

