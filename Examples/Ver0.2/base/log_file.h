#ifndef MUDUO_BASE_LOGFILE_H
#define MUDUO_BASE_LOGFILE_H

#include "Mutex.h"
#include "types.h"

#include <memory>
#include <string>

namespace base {

namespace FileUtil {
class AppendFile;
}

class LogFile : noncopyable {
public:
  LogFile(const std::string &basename, size_t rollSize, bool threadSafe = true,
          bool dupStd = true, int flushInterval = 3, int checkEveryN = 1024);
  ~LogFile();

  void append(const char *logline, int len);
  void flush();
  bool rollFile();

private:
  void append_unlocked(const char *logline, int len);

  static std::string getLogFileName(const std::string &basename, time_t *now);

  const std::string basename_;
  const size_t rollSize_;
  const bool dupStd_;
  const int flushInterval_;
  const int checkEveryN_;

  int count_;

  std::unique_ptr<MutexLock> mutex_;
  time_t startOfPeriod_;
  time_t lastRoll_;
  time_t lastFlush_;
  std::unique_ptr<FileUtil::AppendFile> file_;
  bool forbit_write_;

  const static int kRollPerSeconds_ = 60 * 60 * 24;
};
}
#endif // MUDUO_BASE_LOGFILE_H
