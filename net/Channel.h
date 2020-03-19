/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:23:09
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 15:52:19
 */
#ifndef NET_CHANNEL_H_
#define NET_CHANNEL_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <sys/epoll.h>
#include "Util.h"
#include <iostream>
#include <sys/socket.h>

using namespace std;

struct EventLoop;  // 前向声明

class Channel
{
private:
    typedef std::function<void()> CallBack;
    EventLoop *loop_;
    int fd_;
    __uint32_t events_;
    __uint32_t revents_;
    __uint32_t lastEvents_;
    struct sockaddr_in sock_addr;

private:
    typedef function<void (const int&)> MessageCallback;
    typedef function<void (const struct sockaddr_in&)> WriteCompleteCallback;
    typedef function<void (const struct sockaddr_in&)> ConnectionCallback;
    MessageCallback readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    ConnectionCallback connHandler_;



public:
    Channel(EventLoop *loop);
    Channel(EventLoop *loop, int fd);
    ~Channel();
    int getFd();
    void setFd(int fd);
    void setCliAddr(const sockaddr_in& request)
    {
        memmove(&sock_addr, &request, sizeof(sockaddr_in));
    }

    void setReadHandler(MessageCallback &&readHandler)
    {
        readHandler_ = readHandler;
    }
    void setWriteHandler(CallBack &&writeHandler)
    {
        writeHandler_ = writeHandler;
    }
    void setErrorHandler(CallBack &&errorHandler)
    {
        errorHandler_ = errorHandler;
    }
    void setConnHandler(ConnectionCallback &&connHandler)
    {
        connHandler_ = connHandler;
    }

    void handleEvents()
    {
        events_ = 0;
        if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
        {
            events_ = 0;
            return;
        }
        if (revents_ & EPOLLERR)
        {
            if (errorHandler_) errorHandler_();
            events_ = 0;
            return;
        }
        if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
            handleRead();
        }
        if (revents_ & EPOLLOUT)
        {
            handleWrite();
        }
        // handleConn();  // not implement when not using ONESHUT
    }
    void handleRead();
    void handleWrite();
    void handleError(int fd, int err_num, std::string short_msg);
    void handleConn();
    void wakeFunctor(){}

    void setRevents(__uint32_t ev)
    {
        revents_ = ev;
    }

    void setEvents(__uint32_t ev)
    {
        events_ = ev;
    }
    __uint32_t& getEvents()
    {
        return events_;
    }

    bool EqualAndUpdateLastEvents()
    {
        bool ret = (lastEvents_ == events_);
        lastEvents_ = events_;
        return ret;
    }

    __uint32_t getLastEvents()
    {
        return lastEvents_;
    }

};


#endif /* NET_CHANNEL_H_ */
