#ifndef MUDUO_BASE_ASYNCLOGGING_H
#define MUDUO_BASE_ASYNCLOGGING_H

#include <atomic>
#include <vector>
#include <string>

// #include "blocking_queue.h"
#include "countdown_latch.h"
#include "Mutex.h"
#include "Thread.h"
#include "log_stream.h"

namespace base {

class AsyncLogging : noncopyable {
public:
  AsyncLogging(const std::string &basename, size_t rollSize,
               int flushInterval = 1, bool sync = false, bool dupStd = true);

  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }

  void append(const char *logline, int len);

  void start() {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop() {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

private:
  void threadFunc();

  typedef base::detail::FixedBuffer<base::detail::kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer> > BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int flushInterval_;
  bool sync_;
  bool dupStd_;
  std::atomic<bool> running_;
  std::string basename_;
  size_t rollSize_;
  base::Thread thread_;
  base::CountDownLatch latch_;
  base::MutexLock mutex_;
  base::Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};
}
#endif // MUDUO_BASE_ASYNCLOGGING_H
