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
#include <iostream>

#define MAX_EVENTS 10
#define PORT 3389
#define BUFSIZE 1024

// using namespace std;

void setnonblocking(int sockfd)
{
    int opts = fcntl(sockfd, F_GETFL, 0);
    if (opts < 0)
    {
        perror("get flag.");
        // exit(1);
    }
    opts |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0)
    {
        perror("set flag.");
        // exit(1);
    }
}

int main()
{
    epoll_event oneEvent, events[MAX_EVENTS];
    int listenfd, connfd;
    sockaddr_in servaddr, cliaddr;
    socklen_t addrlen = sizeof servaddr;
    char buf[BUFSIZE];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation.");
//        exit(1);
    }
	int optval = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
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
    int epfd = epoll_create(MAX_EVENTS);
    if (epfd < 0)
    {
        perror("epoll creation.");
    }
    oneEvent.events = EPOLLIN | EPOLLET;  // TODO 测试这个为ET的情况
    oneEvent.data.fd = listenfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &oneEvent) < 0)
    {
        perror("epoll_ctl: listen_sock");
    }

    for (;;)
    {
        int nready = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nready < 0)
        {
            perror("epoll_wait error.");
        }

        for (int i = 0; i <nready; i++)
        {
            int fd = events[i].data.fd;
            if (fd == listenfd)
            {
                if ((connfd = accept(listenfd, reinterpret_cast<sockaddr *>(&servaddr), &addrlen)) > 0)
                {
			std::cout<<"one sock ready"<<std::endl;
		setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, &optval, sizeof optval);

                    memset(&oneEvent, 0, sizeof oneEvent);
                    oneEvent.events = EPOLLIN | EPOLLET;
                    oneEvent.data.fd = connfd;
                    setnonblocking(connfd);
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &oneEvent) < 0)  // 注册accpect到的FD
                    {
                        if (errno != EAGAIN && errno != EPROTO && errno != EINTR)
                            perror("epoll creation.");
                    }
                }
                else
                {
                    perror("accept error.");
                }
            }
            else if (events[i].events & EPOLLIN)
            {
		std::cout<<"read ready"<<std::endl;
                int n =0;
                int nread = 0;
                while ((nread = read(fd, buf + n, 1)) > 0)
                {
		std::cout<<"reading"<<nread<<"bit"<<std::endl;
                    
		break;
		n+= nread;
                }
                if(nread == -1 && errno != EAGAIN)
                {
                    perror("read error");
                }
		else if (nread == 0)
		{
			std::cout<<"socket have been closed!"<<std::endl;
			close(fd);
			epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
		}
                memset(&oneEvent, 0, sizeof oneEvent);
                oneEvent.events = events[i].events | EPOLLOUT;  // TODO 如果这个地方直接放EPOLLOUT， 不与上之前的events会怎样？
                oneEvent.data.fd = fd;
              //  if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &oneEvent) < 0)
              //  {
              //      perror("epoll_ctl: mod");
              //  }
            }
          //  else if (events[i].events & EPOLLOUT)
          //  {
	  //      std::cout<<"ready for EPOLLOUT"<<std::endl;
          //      memset(&buf, 0, sizeof buf);
          //      sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n小杰你好啊:)", 17);  // TODO 测试为什么会一直调用这个write.
          //      int nwrite, datasize = strlen(buf);
          //      int n = datasize;
          //      while (n > 0)
          //      {
          //          nwrite = write(fd, buf + datasize - n, n);
          //          if (nwrite == -1 && errno != EAGAIN)
          //          {
          //              perror("write error");
          //          }
          //          n -= nwrite;
          //      }
          //  }
        }
    }
    return 0;
}

