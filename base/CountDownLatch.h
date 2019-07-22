/*
 * CountDownLatch.h
 *
 *  Created on: Jul 22, 2019
 *      Author: kaiqi
 */

#ifndef BASE_COUNTDOWNLATCH_H_
#define BASE_COUNTDOWNLATCH_H_

#include "base/Condition.h"
#include "base/Mutex.h"

// TODO 目前的理解是相当于一个计数器，只有在所有的线程都已经做好了（countdown）,才去signal all， 从wait里面出来，进行下面的步骤
class CountDownLatch : noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};


#endif /* BASE_COUNTDOWNLATCH_H_ */
