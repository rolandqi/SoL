/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:23:15
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 11:40:37
 */

#include <unistd.h>
#include <queue>
#include <cstdlib>
#include <iostream>
#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"


Channel::Channel(EventLoop *loop):
    loop_(loop),
    events_(0),
    lastEvents_(0)
{ }

Channel::Channel(EventLoop *loop, int fd):
    loop_(loop),
    fd_(fd),
    events_(0),
    lastEvents_(0)
{ }

Channel::~Channel()
{
    //loop_->poller->epoll_del(fd, events_);
    //close(fd_);
}

int Channel::getFd()
{
    return fd_;
}
void Channel::setFd(int fd)
{
    fd_ = fd;
}

void Channel::handleRead()
{
    if (readHandler_)
    {
        readHandler_(fd_);
    }
}

void Channel::handleWrite()
{
    if (writeHandler_)
    {
        writeHandler_();
    }
}

void Channel::handleConn()
{
    if (connHandler_)
    {
        connHandler_(sock_addr);
    }
}
