/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-18 10:08:08
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 16:59:06
 */
#include "net/EventLoop.h"
#include "net/Client.h"

const int PORT = 8421;
const int MAX_SIZE = 20;

struct echoContent
{
    char size;
    char body[MAX_SIZE];
};

void echoClientRead(int fd)
{
    char size = 0;
    int nread = readn(fd, &size, 1);  // read out the lenth of the data.
    if (nread != 1)
    {
        LOG_INFO << "Receiving data error in fd: " << fd;
    }
    char* buffer = new char[size];
    memset(buffer, 0, size);
    nread = readn(fd, buffer, size);
    if (size > 0)
    {
        LOG_INFO << "receiving data size: "<< static_cast<int>(size);
        LOG_INFO << "Receiving data :" << buffer;
    }
    if (nread != size)
    {
        LOG_INFO << "Receiving data error in fd: " << fd;
    }
    delete buffer;
}

void echoClientConnection(int fd)
{
    LOG_INFO << "Server connected!";
    echoContent myContent;
    myContent.size = 6;
    memmove(&myContent.body, "HELLO", 6);
    // myContent.body = "HELLO";
    char size = 6;

    // int nwrite = writen(fd, &size, 1);
    // if (nwrite != 1)
    // {
    //     LOG_INFO << "write size failed! size : " << static_cast<int>(size);
    // }
    // char buffer[MAX_SIZE] = "HELLO";
    // nwrite = writen(fd, reinterpret_cast<char*>(&buffer), static_cast<int>(size));
    int nwrite = writen(fd, reinterpret_cast<char*>(&myContent), static_cast<int>(myContent.size) + 1);
    if (nwrite != size + 1)
    {
        LOG_INFO << "write failed!";
    }
}

int main() {
    EventLoop loop;
    Client myClient(&loop, 1, PORT);  // 1 worker.
    myClient.setConnectionCallback(bind(echoClientConnection, _1));
    myClient.setMessageCallback(bind(echoClientRead, _1));
    myClient.start();
    loop.loop();
}