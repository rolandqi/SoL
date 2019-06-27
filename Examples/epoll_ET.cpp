/*
 * epoll_ET.cpp
 *
 *  Created on: Jun 27, 2019
 *      Author: kaiqi
 */
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>


#define MAX_EVENTS 10
#define PORT 8080
#define BUFSIZE 1024

void setnonblocking(int sockfd)
{
    int opts = fcntl(sockfd, F_GETFL, 0);
    if (opts < 0)
    {
        perror("get flag.");
        // exit(1);
    }
    opts |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flag) < 0)
    {
        perror("set flag.");
        // exit(1);
    }
}

int main()
{
    epoll_event oneEvent, events[MAX_EVENTS];
    int addrlen, listenfd, connfd;
    sockaddr_in servaddr, cliaddr;
    addrlen = sizeof sockaddr_in;
    char buf[BUFSIZE];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation.");
//        exit(1);
    }
    setnonblocking(listenfd);

    memset(&servaddr, 0,  sizeof(sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, reinterpret_cast<sockaddr*>(&servaddr), addrlen) < 0)
    {
        perror("Bind error.");
    }
    if (listen(listenfd, 1024) < 0)
    {
        perror("Listen error.");
    }
    int epfd = epoll_create(MAX_EVENT);
    if (epfd < 0)
    {
        perror("epoll creation.");
    }
    oneEvent.events = EPOLLIN;
    oneEvent.data.fd = listenfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &oneEvent) < 0)
    {
        perror("epoll_ctl: listen_sock");
    }

    for (;;)
    {
        int nready = epoll_wait(epfd, events, MAX_EVENT, -1);
        if (nready < 0)
        {
            perror("epoll_wait error.");
        }

        for (int i = 0; i <nready; i++)
        {
            int fd = events[i].data.fd;
            if (fd == listenfd)
            {
                if ((connfd = accept(listenfd, reinterpret_cast<sockaddr *>(servaddr), &addrlen)) > 0)
                {
                    memset(&oneEvent, 0, sizeof oneEvent);
                    oneEvent.evnets = EPOLLIN | EPOLLET;
                    oneEvnet.data.fd = connfd;
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &oneEvent) < 0)  // 注册accpect到的FD
                    {
                        if (errno != EAGAIN && errno != ECONNABORTD && errno != EPROTO && errno != EINTR)
                            perror("epoll creation.");
                    }
                }
                else
                {
                    perror("accept error.");
                }
            }
            else if (events[i].enents & EPOLLIN)
            {
                int n =0;
                int nread = 0;
                while ((nread = read(fd, buf + n, BUFSIZE - 1)) > 0)
                {
                    n+= nread;
                }
                if(nread == -1 && errno != EAGAIN)
                {
                    perror("read error");
                }
                memset(&oneEvent, 0, sizeof oneEvent);
                oneEvent.evnets = events[i].evnets | EPOLLOUT;
                oneEvnet.data.fd = fd;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, &oneEvent) < 0)
                {
                    perror("epoll_ctl: mod");
                }
            }
            else if (event[i].enent == EPOLLOUT)
            {
                memset(&buf, 0, sizeof buf);
                sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nHello World", 11);
                int nwrite, datasize = strlen(buf);
                int n = datasize;
                while (n > 0)
                {
                    nwrite = write(fd, buf + data_size - n, n);
                    if (nwrite == -1 && errno != EAGAIN)
                    {
                        perror("write error");
                    }
                    n -= nwrite;
                }
            }
        }
    }
    return 0;
}



