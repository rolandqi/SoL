/*
 * epoll_client.cpp
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

#define MAXSIZE = 1024
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
void moidfy_event(int epollfd, int fd, int state);

int count = 0;

int main (int argc, char*argv[])
{
    int sockfd;
    sockaddr_in servaddr;
    int listenfd = sock(AF_INET, SOCKET_STREAM, 0);
    bzero(&servaddr, sizeof servaddr);
    inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
    servaddr.sin_family = 
    
}