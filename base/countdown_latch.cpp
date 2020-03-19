// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "countdown_latch.h"

using namespace base;

CountDownLatch::CountDownLatch(int count)
    : mutex_(), condition_(mutex_), count_(count) {}

void CountDownLatch::wait() {
  base::MutexLockGuard lock(mutex_);
  while (count_ > 0) {
    condition_.wait();
  }
}

void CountDownLatch::countDown() {
  base::MutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0) {
    condition_.notifyAll();
  }
}

int CountDownLatch::getCount() const {
  base::MutexLockGuard lock(mutex_);
  return count_;
}
