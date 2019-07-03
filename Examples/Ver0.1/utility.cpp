#include "utility.h"
#include <signal.h>
#include <fcntl.h>

void handle_for_sigpipe()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, nullptr);
}

int setSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}

ssize_t writen(int sockfd, const void* buf, int length)
{
  int written = 0;
  int retrycount = AGAIN_MAX_TIMES;
  while (written < length)
  {
    ssize_t nw = ::write(sockfd, static_cast<const char*>(buf) + written, length - written);
    if (nw > 0)
    {
        written += static_cast<int>(nw);
    }
    else if (nw == 0)
    {
        break;  // EOF
    }
    else if (errno != EINTR && errno != EAGAIN)
    {
        perror("write");
        // 有可能出现数据读到一半时出现err的情况
        // 这种情况抛弃所有的data
        return -1;
    }
    else
    {
        if (retrycount-- > 0)
        {
            continue;   // 采用这种简陋的方式处理EINTR和EAGAIN的情况
        }
        else
        {
            return -1;
        }
        
    }
  }
  return written;
}

ssize_t readn(int sockfd, void* buf, int length)
{
  int nread = 0;
  int retrycount = 10;
  while (nread < length)
  {
    ssize_t nr = ::read(sockfd, static_cast<char*>(buf) + nread, length - nread);
    if (nr > 0)
    {
        nread += static_cast<int>(nr);
    }
    else if (nr == 0)
    {
        break;  // EOF
    }
    else if (errno != EINTR && errno != EAGAIN)
    {
        perror("read");
        break;
    }
    else
    {
        if (retrycount-- > 0)
        {
            continue;   // 采用这种简陋的方式处理EINTR和EAGAIN的情况
        }
        else
        {
            break;
        }
        
    }
    
  }
  return nread;
}