
#include <stdio.h>

#include <assert.h>

#include <stdlib.h>

#include <unistd.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <string.h> 

#include <sys/socket.h>

#include <sys/types.h>

#include <sys/stat.h>

#include <sys/epoll.h>

 


/* 事件数组最大值 */

#define MAX_EVENTS 256

 

/* 每一个客户端在事件集中 events.data.ptr 所指向的数据 */

typedef struct fd_buf

{

	int  fd;        /* 保存当前客户端的socket描述符 */

	char buf[1024]; /* 当前客户端的缓冲区 */

}fd_buf_t, *fd_buf_p;

 

/* 为每一个客户端动态分配fd_buf_t 结构体 */

static void* new_fd_buf(int fd);

 

/* epoll事件集 */

struct epoll_event evts[MAX_EVENTS];

 

/* 获取一个监听连接的sockfd */

int run_getsockfd(const char*ip, int port);

 

/* 执行epoll逻辑 (创建，添加监听事件，等待返回)*/

void run_epoll(int listen_sockfd);

 

/* 处理客户端事件就绪 */

void run_action(int epollfd, int index);

 

/* 处理客户端连接请求，并添加到epoll模型中 */

void run_accept(int epollfd, int listen_sockfd);

 

 

int main(int argc, char** argv)

{
	/* 使用格式 ./xxx  服务器ip  服务器端口 */

	if(argc != 3)

	{

		printf("usage: [server_ip] [server_port]\n");

		return 1;

	}

 

	/* 获取监听socketfd  */

	int listen_sockfd = run_getsockfd(argv[1], atoi(argv[2]));

 

	/* 执行epoll 逻辑 */

	run_epoll(listen_sockfd);

 

	return 0;

}

 

 

/* 获取一个监听socket */

int run_getsockfd(const char* ip, int port)

{

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if( sock < 0){

		perror("socket()");

		exit(1);

	}

 

	/* 复用*/

	int opt = 1;

	setsockopt(sock , SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

 

	struct sockaddr_in server;

	bzero(&server, sizeof(server));

	server.sin_addr.s_addr = inet_addr(ip);

	server.sin_port = htons(port);

	server.sin_family = AF_INET;

 

	if(bind(sock, (struct sockaddr *)&server, sizeof(server) ) < 0){

		perror("bind()");

		exit(2);

	}

 

	if(listen(sock, 5) < 0){

		perror("listen()");

		exit(3);

	}

 

	return sock;

}

 

 

 

/* 执行epoll逻辑 (创建，添加监听事件，等待返回)*/

void run_epoll(int listen_sockfd)

{

	// 1. 创建 epoll 模型

	int epollfd = epoll_create(256);

	if( epollfd < 0)

	{

		perror("epoll_create()");

		exit(4);

	}

 

	// 2. 添加监听描述符到模型中，并关心读事件

	struct epoll_event evt;

	evt.events = EPOLLIN;

	evt.data.ptr = new_fd_buf(listen_sockfd);

	epoll_ctl(epollfd, EPOLL_CTL_ADD,listen_sockfd, &evt);

 

	int nfds = 0;        /* 就绪事件个数 */

	int timeout = 1000;  /* 一秒超时  */

	while(1)

	{

		nfds = epoll_wait(epollfd, evts, MAX_EVENTS, timeout );	

		if( nfds < 0)

		{

			perror("epoll_wait()");

			continue;

		}

		else if (nfds == 0)

		{

			printf("epoll_wait() timeout\n");

		}

		else

		{

			/* 变量处理已经就绪的事件 */

			int idx_check = 0;

			for(; idx_check < nfds; idx_check++)

			{

				fd_buf_p fp  = evts[idx_check].data.ptr;

				/* 监听socket 读事件就绪 */

				if( fp->fd == listen_sockfd &&\

					evts[idx_check].events & EPOLLIN )

				{

					/* 接受客户端的请求 */

					run_accept(epollfd, listen_sockfd);

				}/* 客户端socket 事件就绪 */

				else if ( fp->fd != listen_sockfd )

				{

					run_action(epollfd, idx_check);

				}

			}

		}

	}

}

 

static void* new_fd_buf(int fd)

{

	fd_buf_p ret  = (fd_buf_p)malloc(sizeof(fd_buf_t));

	assert( ret != NULL);

 

	ret->fd = fd;

	memset(ret->buf, 0, sizeof(ret->buf));

	return ret; 

}

 

/* 处理客户端连接请求，并添加到epoll模型中 */

void run_accept(int epollfd, int listen_sockfd)

{

	

	struct sockaddr_in cliaddr;

	socklen_t clilen = sizeof(cliaddr);

 

	int new_sock = accept(listen_sockfd, (struct sockaddr*)&cliaddr, &clilen);

	if(new_sock < 0)

	{

		perror("accept");

		return;

	}

 

	printf("与客户端连接成功: ip %s port %d \n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

 

	/* 将与客户端连接的sockfd 添加到epoll模型中，并关心读事件 */

	struct epoll_event evt;

	evt.events = EPOLLIN;

	evt.data.ptr = new_fd_buf(new_sock); /* 为每一个客户端链接分配fd和缓冲区 */

	epoll_ctl(epollfd, EPOLL_CTL_ADD, new_sock ,&evt);

}

 

/* 处理客户端事件就绪 */

void run_action(int epollfd, int index)

{

 

	/* 取得客户端结构体数据指针 */

	fd_buf_p fdp = evts[index].data.ptr;

 

	/* 读事件就绪 */

	if(evts[index].events & EPOLLIN)

	{

		ssize_t s = read(fdp->fd, fdp->buf, sizeof(fdp->buf));

		if(s > 0)

		{

			fdp->buf[s] = 0;

			printf("客户端请求消息:\n");

			printf("\n %s \n", fdp->buf);

			struct epoll_event evt; 

			evt.events = EPOLLOUT;

			evt.data.ptr = fdp;

			epoll_ctl(epollfd, EPOLL_CTL_MOD, fdp->fd, &evt);

		}

		else if(s <= 0)

		{

			/* 记得关闭socket，在epoll 模型中删除结点并释放分配的内存 */

			printf("\n客户端退出!\n");

			close(fdp->fd);

			epoll_ctl(epollfd, EPOLL_CTL_DEL, fdp->fd, NULL );

			free(fdp);

		}

	}

	else if(evts[index].events & EPOLLOUT)

	{/* 写事件就绪 */

		/* 发送完毕后，记得做清理操作 */

		const char* msg = "HTTP/1.1 200 OK\r\n\r\n<html><h1>Hello epoll!</h1></html>";

		write(fdp->fd, msg, strlen(msg));

		close(fdp->fd);

		epoll_ctl(epollfd, EPOLL_CTL_DEL, fdp->fd, NULL);

		free(fdp);

	}
}
