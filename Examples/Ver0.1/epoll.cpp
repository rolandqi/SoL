#include "epoll.h"
#include <stdio.h>

using namespace std;

struct epoll_event* events;  // 在这里声明全局变量 TODO why???

int epoll_init()
{
    int fd = epoll_create(MAXEVENTS);
    if (fd < 0)
    {
        perror("epoll creation.");
        return -1;
    }
    events = new epoll_event[MAXEVENTS];
    return fd;
}

int epoll_add(int epollfd, int fd, void *request, UINT_32 events)
{
    epoll_event event;
    event.data.ptr = request;
    event.events = events;
    if (::epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll add");
        return -1;
    }
    return 0;
}

int epoll_modify(int epollfd, int fd, void *request, UINT_32 events)
{
    epoll_event event;
    event.data.ptr = request;
    event.events = events;
    if (::epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) < 0)
    {
        perror("epoll modify");
        return -1;
    }
    return 0;
}

int epoll_delete(int epollfd, int fd, void *request, UINT_32 events)
{
    epoll_event event;
    event.data.ptr = request;
    event.events = events;
    if (::epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) < 0)
    {
        perror("epoll delete");
        return -1;
    }
    return 0;
}
int epoll_waits(int epollfd, int fd, epoll_event *events, int max_events, int timeout)
{
    int nready = epoll_wait(epollfd, events, max_events, timeout);
    if (nready < 0)
    {
        perror("epoll wait error");
    }
    return nready;
}
