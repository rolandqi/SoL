#include <net/Client.h>
#include <base/logging.h>
#include <base/Thread.h>
#include <net/EventLoop.h>
#include <net/EventLoopThreadPool.h>
#include <base/Atomic.h>
// #include <boost/bind.hpp>
// #include <boost/ptr_container/ptr_vector.hpp>
#include <functional>
#include <memory>

#include <utility>

#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <memory>

#define MAX_SIZE 1024


class PingpongClient;
struct PingPongContent
{
    char size;
    char body[MAX_SIZE];
};

class Session
{
 public:
  Session(EventLoop* loop,
          int port,
          const string& name,
          PingpongClient* owner)
    : client_(loop, 1, port),
      owner_(owner),
      bytesRead_(0),
      bytesWritten_(0),
      messagesRead_(0)
  {
    client_.setConnectionCallback(
        std::bind(&Session::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&Session::onMessage, this, _1));
    start();
  }

  void start()
  {
    client_.start();
  }

  void stop()
  {
    client_.~Client();
  }

  int64_t bytesRead() const
  {
     return bytesRead_;
  }

  int64_t messagesRead() const
  {
     return messagesRead_;
  }

 private:

  void onConnection(const int& fd);

  void onMessage(int fd);

  Client client_;
  PingpongClient* owner_;
  int64_t bytesRead_;
  int64_t bytesWritten_;
  int64_t messagesRead_;
};

class PingpongClient
{
 public:
  PingpongClient(EventLoop* loop,
         int port,
         int blockSize,
         int sessionCount,
         int timeout,
         int threadCount)
    : loop_(loop),
      threadPool_(loop),
      blockSize_(blockSize),
      sessionCount_(sessionCount),
      timeout_(timeout)
  {
    loop->runAfter(timeout, std::bind(&PingpongClient::handleTimeout, this));
    if (threadCount > 1)
    {
      threadPool_.setThreadNum(threadCount);
    }
    threadPool_.start();

    for (int i = 0; i < blockSize; ++i)
    {
      message_.push_back(static_cast<char>(i % 128));
    }

    for (int i = 0; i < sessionCount; ++i)
    {
      char buf[32];
      snprintf(buf, sizeof buf, "C%05d", i);
      shared_ptr<Session> session(threadPool_.getNextLoop(), port, buf, this);
      session->start();
      sessions_.push_back(session);
    }
  }

  const string& message() const
  {
    return message_;
  }

  const int blockSize() const
  {
    return blockSize_;
  }

  void onConnect()
  {
    if (numConnected_.incrementAndGet() == sessionCount_)
    {
      LOG_WARN << "all connected";
    }
  }

  void onDisconnect(const int& fd)
  {
    if (numConnected_.decrementAndGet() == 0)
    {
      LOG_WARN << "all disconnected";

      int64_t totalBytesRead = 0;
      int64_t totalMessagesRead = 0;
      for (auto it = sessions_.begin(); it != sessions_.end(); ++it)
      {
        totalBytesRead += it->bytesRead();
        totalMessagesRead += it->messagesRead();
      }
      LOG_WARN << totalBytesRead << " total bytes read";
      LOG_WARN << totalMessagesRead << " total messages read";
      LOG_WARN << static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead)
               << " average message size";
      LOG_WARN << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024)
               << " MiB/s throughput";
      conn->getLoop()->queueInLoop(std::bind(&Client::quit, this));
    }
  }

 private:

  void quit()
  {
    loop_->queueInLoop(std::bind(&EventLoop::quit, loop_));
  }

  void handleTimeout()
  {
    LOG_WARN << "stop";
    std::for_each(sessions_.begin(), sessions_.end(),
                  std::mem_fn(&Session::stop));
  }

  EventLoop* loop_;
  EventLoopThreadPool threadPool_;
  int sessionCount_;
  int timeout_;
  std::vector<std::stared_ptr<Session>> sessions_;
  string message_;
  int blockSize_;
  AtomicInt32 numConnected_;
};

void Session::onConnection(const int& fd)
{
    LOG_INFO << "Server connected!";
    owner_->onConnect();
    PingPongContent myContent;
    myContent.size = owner_->blockSize();
    memmove(&myContent.body, owner_->message().c_str(), owner_->blockSize());

    int nwrite = writen(fd, reinterpret_cast<char*>(&myContent), static_cast<int>(myContent.size) + 1);
    if (nwrite != myContent.size + 1)
    {
        LOG_INFO << "write failed!";
    }
}

int main(int argc, char* argv[])
{
  if (argc != 7)
  {
    fprintf(stderr, "Usage: client <port> <threads> <blocksize> ");
    fprintf(stderr, "<sessions> <time>\n");
  }
  else
  {
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    int threadCount = atoi(argv[2]);
    int blockSize = atoi(argv[3]);
    int sessionCount = atoi(argv[4]);
    int timeout = atoi(argv[5]);

    EventLoop loop;

    PingpongClient client(&loop, port, blockSize, sessionCount, timeout, threadCount);
    loop.loop();
  }
}

  void Session::onMessage(int fd)
  {
    PingPongContent myContent;
    // receive data: 1 byte length + data.
    int nread = readn(fd, &myContent.size, 1);  // read out the lenth of the data.
    if (nread < 0)
    {
        LOG_INFO << "Receiving data error in fd: " << fd;
        owner_->onDisconnect(fd);
    }
    nread = readn(fd, myContent.body, myContent.size);
    if (myContent.size > 0)
    {
        LOG_INFO << "receiving data size: "<< static_cast<int>(myContent.size);
    }
    if (nread != myContent.size)
    {
        LOG_INFO << "Receiving data error in fd: " << fd;
    }

    // echo back
    int nwrite = writen(fd, reinterpret_cast<char*>(&myContent), static_cast<int>(myContent.size) + 1);
    if (nwrite != myContent.size + 1)
    {
        LOG_INFO << "write failed! write size: " << nwrite;
    }
    bytesWritten_ += myContent.size;
    messagesRead_++;
    bytesRead_ += myContent.size;
  }