/*
 * epoll_client.cpp
 *
 *  Created on: June 27, 2019
 *      Author: kaiqi
 */
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

#define MAXSIZE 1024
#define IPADDRESS "127.0.0.1"
#define SERV_PORT 1234
#define FD_SIZE 1024
#define EPOLLEVENTS 20

void handle_connection(int sockfd);
void handle_events(int epollfd, struct epoll_event * events, int num, int sockfd, char* buf);
void do_read(int epollfd, int fd, int sockfd, char *buf);
void do_write(int epollfd, int fd, int sockfd, char *buf);
void add_event(int epollfd, int fd, int state);
void delete_event(int epollfd, int fd, int state);
void modify_event(int epollfd, int fd, int state);
void setnonblocking(int sockfd);

int count = 0;

int main (int argc, char*argv[])
{
    sockaddr_in servaddr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof servaddr);
    inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
    servaddr.sin_family = AF_INET;
    connect(sockfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof servaddr);   // client 连接到服务器，三次握手成功之后才返回
    handle_connection(sockfd);
    close(sockfd);
    return 0;
}

void handle_connection(int sockfd)
{
    epoll_event events[EPOLLEVENTS];
    memset(&events, 0, sizeof(epoll_event) * EPOLLEVENTS);
    char buf[MAXSIZE];
    int epollfd = epoll_create(EPOLLEVENTS);
    add_event(epollfd, sockfd, EPOLLIN);
    while (1)
    {
        int nready = epoll_wait(epollfd, events, EPOLLEVENTS, -1);  // 注意，这个地方调用的不是&events 而是 events， 因为events是一个数组
        handle_events(epollfd, events, nready, sockfd, buf);
    }
    close(epollfd);
}

void handle_events(int epollfd, struct epoll_event *events, int nready, int sockfd, char *buf)
{
    for (int i = 0; i < nready; i++)
    {
        int fd = events[i].data.fd;
        if (events[i].events & EPOLLIN)
        {
            do_read(epollfd, fd, sockfd, buf);
        }
        if(events[i].events & EPOLLOUT)
        {
            do_write(epollfd, fd, sockfd, buf);
        }
    }
}

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
    if (fcntl(sockfd, F_SETFL, opts) < 0)
    {
        perror("set flag.");
        // exit(1);
    }
}

void do_read(int epollfd, int fd, int sockfd, char *buf)
{
    int nread = read(fd, buf, MAXSIZE);
    if (nread == -1)
    {
        if (errno != EAGAIN)
        {
            perror("read error");
            close(fd);  //TODO 如果一个句柄已经注册在epoll rbtree里面了，那么能否直接关闭?
        }
    }
    else if (nread == 0)
    {
        perror("read but server closed!");
        close(fd);
    }
    else
    {
        if (fd == STDIN_FILENO)
        {
            add_event(epollfd, sockfd, EPOLLOUT);  // 直到读取到键盘输入之后，才把clinet socket注册到epoll系统中
        }
        else
        {
            delete_event(epollfd, sockfd, EPOLLIN);
            add_event(epollfd, sockfd, EPOLLOUT);
        }
    }
}

void do_write(int epollfd, int fd, int sockfd, char *buf)
{
    char temp[MAXSIZE];
    buf[strlen(buf) + 1] = '\0';
    sprintf(temp, "%s_%d", buf, ++count);
    int nwrite = write(fd, temp, strlen(temp));
    if (nwrite == -1 && errno != EAGAIN)
    {
        perror("write error！");
    }
    else
    {
        if (fd == STDOUT_FILENO)
        {
            delete_event(epollfd, fd, EPOLLOUT);
        }
        else
        {
            modify_event(epollfd, fd, EPOLLIN);
        }
    }
    memset(&buf, 0, MAXSIZE);
}

void add_event(int epollfd, int fd, int status)
{
    struct epoll_event ev;
    ev.events = status;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void delete_event(int epollfd, int fd, int status)
{
    struct epoll_event ev;
    ev.events = status;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void modify_event(int epollfd, int fd, int status)
{
    struct epoll_event ev;
    ev.events = status;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}



















