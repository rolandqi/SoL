#include <net/Server.h>

#include <base/logging.h>
#include <base/Thread.h>
#include <net/EventLoop.h>

#include <functional>

#include <utility>

#include <stdio.h>
#include <unistd.h>
const int MAX_SIZE = 10240;
using std::placeholders::_1;
using std::placeholders::_2;
struct PingPongContent
{
    char size;
    char body[MAX_SIZE];
};

void onConnection(const sockaddr_in& request)
{
  LOG_INFO << "Connect one client from port: " << request.sin_port;
}

void onMessage(const int& fd)
{
    PingPongContent myContent;
    // receive data: 1 byte length + data.
    int nread = readn(fd, &myContent.size, 1);  // read out the lenth of the data.
    if (nread != 1)
    {
        LOG_INFO << "Receiving data error in fd: " << fd;
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
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: server <port> <threads>\n");
  }
  else
  {
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    int threadCount = atoi(argv[2]);

    EventLoop loop;

    Server server(&loop, threadCount, port);

    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
  }
}

