#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "utility.h"

const int MAXEVENTS = 5000;
const int LISTENQ = 1024;


int epoll_init();
int epoll_add(int epollfd, int fd, void *request, UINT_32 events);
int epoll_modify(int epollfd, int fd, void *request, UINT_32 events);
int epoll_delete(int epollfd, int fd, void *request, UINT_32 events);
int epoll_waits(int epollfd, int fd, epoll_event *events, int max_events, int timeout);

#endif