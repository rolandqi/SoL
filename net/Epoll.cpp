/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:22:57
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 11:31:07
 */
#include "Epoll.h"
#include <assert.h>
#include <iostream>

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll():
    epollFd_(epoll_create1(EPOLL_CLOEXEC)),
    events_(EVENTSNUM)
{
    assert(epollFd_ > 0);
}
Epoll::~Epoll()
{ }


// 注册新描述符
void Epoll::epoll_add(SP_Channel request, int timeout)
{
    int fd = request->getFd();
    if (timeout > 0)
    {
        add_timer(request, timeout);
    }
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    // LOG_INFO << "epoll add fd " << fd << " events: " << event.events;

    request->EqualAndUpdateLastEvents();

    fd2chan_[fd] = request;
    if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        fd2chan_[fd].reset();
    }
}


// 修改描述符状态
void Epoll::epoll_mod(SP_Channel request, int timeout)
{
    if (timeout > 0)
        add_timer(request, timeout);
    int fd = request->getFd();
    if (!request->EqualAndUpdateLastEvents())
    {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->getEvents();
        if(epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0)
        {
            perror("epoll_mod error");
            fd2chan_[fd].reset();
        }
    }
}


// 从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request)
{
    int fd = request->getFd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getLastEvents();
    //event.events = 0;
    // request->EqualAndUpdateLastEvents()
    if(epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
    }
    fd2chan_[fd].reset();
}

// 返回活跃事件数
void Epoll::poll(int timeoutMs, vector<SP_Channel>* activeChannels)
{
    int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), timeoutMs);
    if (event_count < 0)
        perror("epoll wait error");
    std::vector<shared_ptr<Channel>> requests = getEventsRequest(event_count);
    activeChannels->assign(requests.begin(), requests.end());
}

void Epoll::handleExpired()
{
    // TODO handle expire events.
    // timerManager_.handleExpiredEvent();
}

// 分发处理函数
std::vector<shared_ptr<Channel>> Epoll::getEventsRequest(int events_num)
{
    std::vector<SP_Channel> req_data;
    for(int i = 0; i < events_num; ++i)
    {
        // 获取有事件产生的描述符
        int fd = events_[i].data.fd;

        SP_Channel cur_req = fd2chan_[fd];

        if (cur_req)
        {
            cur_req->setRevents(events_[i].events);
            cur_req->setEvents(0);
            req_data.push_back(cur_req);
        }
        else
        {
            LOG_INFO << "SP cur_req is invalid";
        }
    }
    return req_data;
}

void Epoll::add_timer(SP_Channel request_data, int timeout)
{
    // TODO: adding timer mechenism to TCP link.
    // if (request_data)  
    //     timerManager_.addTimer(t, timeout);
    // else
    //     LOG_INFO << "timer add fail";
}
