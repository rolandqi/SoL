#include "log_file.h"

#include "file_util.h"
#include "process_info.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string>

using namespace base;

LogFile::LogFile(const std::string &basename, size_t rollSize, bool threadSafe,
                 bool dupStd, int flushInterval, int checkEveryN)
    : basename_(basename), rollSize_(rollSize), dupStd_(dupStd),
      flushInterval_(flushInterval), checkEveryN_(checkEveryN), count_(0),
      mutex_(threadSafe ? new MutexLock : NULL), startOfPeriod_(0),
      lastRoll_(0), lastFlush_(0), forbit_write_(false) {
  rollFile();
}

LogFile::~LogFile() {}

void LogFile::append(const char *logline, int len) {
  if (mutex_) {
    base::MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
  } else {
    append_unlocked(logline, len);
  }
}

void LogFile::flush() {
  if (mutex_) {
    base::MutexLockGuard lock(*mutex_);
    file_->flush();
  } else {
    file_->flush();
  }
}

void LogFile::append_unlocked(const char *logline, int len) {
  if (forbit_write_) {
    file_->DummyIncWrittenBytes(len);
  } else {
    file_->append(logline, len);
  }

  if (file_->writtenBytes() > rollSize_) {
    rollFile();
  } else {
    ++count_;
    if (count_ >= checkEveryN_) {
      count_ = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_) {
        rollFile();
      } else if (now - lastFlush_ > flushInterval_) {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

bool LogFile::rollFile() {
  time_t now = 0;
  std::string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_) {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new FileUtil::AppendFile(filename));
    long available_size = FileUtil::GetAvailableSpace(filename.c_str());
    if (available_size < (300 << 20)) { // 小于300M 不会再写文件
      forbit_write_ = true;
      fprintf(stderr,
              "filesystem available size less than 300M, forbit write\n");
    } else {
      forbit_write_ = false;
    }
    // 将STDOUT_FILENO和STDERR_FILENO也重定向到这个文件
    FILE *fp = file_->fp();
    if (dupStd_) {
      dup2(fileno(fp), STDOUT_FILENO);
      dup2(fileno(fp), STDERR_FILENO);
    }
    return true;
  }
  return false;
}

std::string LogFile::getLogFileName(const std::string &basename, time_t *now) {
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  // gmtime_r(now, &tm); // FIXME: localtime_r ?
  localtime_r(now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  filename += ProcessInfo::hostname();

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}
