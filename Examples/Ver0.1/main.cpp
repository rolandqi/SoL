/*
 * main.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: kaiqi
 */

#include "utility.h"
#include "epoll.h"
#include "HttpData.h"
#include "threadpool.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>  // sockaddr_in


using namespace std;

const int MAX_EVENTS = 10;
const int PORT = 3389;
const int BUFSIZE = 1024;

const int THREADPOOL_THREAD_NUM = 2;
const int QUEUE_SIZE = 128;

const int ASK_STATIC_FILE = 1;
const int ASK_IMAGE_STITCH = 2;

const string PATH = "/";
const int TIMER_TIME_OUT = 500;

priority_queue<mytimer*, vector<mytimer*>, comparator> myTimerQueue;

int socket_bind_listen(const int port)
{
    if (port < 1024 || port >= 65536)
    {
        return -1;
    }
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket creation error.");
        return -1;
    }

    // 消除bind时"Address already in use"错误
    int optval = 1;
    if(setsockopt(listenfd, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) == -1)  // TODO 记录一下这个函数
    {
        perror("setsockopt error!");
        return -1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)) == -1)
    {
        perror("bind Error!");
        return -1;
    }

    if(listen(listenfd, LISTENQ) == -1)
    {
        perror("listen Error!");
        return -1;
    }

    return listenfd;
}

void acceptConnection(int listen_fd, int epoll_fd, const string &path)
{
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t cliaddr_len = sizeof cliaddr;
    int acceptfd = 0;
    while ((acceptfd = accept(listen_fd, reinterpret_cast<sockaddr*> (&cliaddr), &cliaddr_len)) > 0)
    {
        if (setSocketNonBlocking(acceptfd) < 0)
        {
            perror("set non blocking error.");
        }
        //这个地方为什么要用one shot？ 接受了一次数据之后，发送给客户端一次，在发送完成之后，在request_data里面再把fd变成EPOLLIN+ONESHUT
        requestData *req_info = new requestData(epoll_fd, acceptfd, path);
        epoll_add(epoll_fd, acceptfd, reinterpret_cast<void *>(req_info), EPOLLIN | EPOLLET | EPOLLONESHOT);
        mytimer *mtimer = new mytimer(req_info, TIMER_TIME_OUT);
        req_info->addTimer(mtimer);
        myTimerQueue.push(mtimer);
    }
}

void myHandler(void *args)
{
    requestData *req_data = (requestData*)args;
    req_data->handleRequest();
}

void handle_events(int epoll_fd, int listen_fd, struct epoll_event* events, int events_num, const string &path, threadpool_t* tp)
{
    for(int i = 0; i < events_num; i++)
    {
        requestData *request = reinterpret_cast<requestData*> (events[i].data.ptr);
        int fd = request->getFd();
        
        if (fd == listen_fd)
        {
            acceptConnection(listen_fd, epoll_fd, path);
        }
        else
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                perror("error events");
                delete request;  // 因为你是oneshut， 所以可以delete
                continue;
            }

            // 将请求任务加入到线程池中
            // 加入线程池之前将Timer和request分离
            request->seperateTimer();
            int rc = threadpool_add(tp, myHandler, request, 0);
        }   
    }
}

/* 处理逻辑是这样的:
因为(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为deleted的时间节点，会延迟到它(1)超时 或 (2)它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
(1) 第一个好处是不需要遍历优先队列，省时。
(2) 第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请requestData节点了，这样可以继续重复利用前面的requestData，减少了一次delete和一次new的时间。
*/
void handle_expired_event()
{
    while (!myTimerQueue.empty())
    {
        mytimer *ptimer = myTimerQueue.top();
        if (ptimer->isDeleted())  // 如果已经加入线程池了，那么这个timer就可以删除了
        {
            myTimerQueue.pop();
            delete ptimer;  // 之所以能在这里delete掉requestdata, 是因为timeout设置时间都很长，（并且用了oneshut），做一次之后，timeout结束之后就要删除
        }
        else if (ptimer->isvalid() == false)
        {
            myTimerQueue.pop();
            delete ptimer;
        }
        else
        {
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    epoll_event* events = new epoll_event[MAXEVENTS];
    // handle for sigpipe
    int epollfd = epoll_init(events);  // 在里面初始化epoll的返回指针 events的空间。
    if (epollfd < 0)
    {
        perror("epoll creation failed.");
        return 1;
    }

    threadpool_t* threadpool = threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE, 0);
    if (threadpool == nullptr)
    {
        perror("thread pool creation error.");
        return 1;
    }

    int listen_fd = socket_bind_listen(PORT);
    if (listen_fd < 0)
    {
        perror("socket bind failed");
        return 1;
    }
    if (setSocketNonBlocking(listen_fd) < 0)
    {
        perror("set socket non block failed");
        return 1;
    }
    UINT_32 event = EPOLLIN | EPOLLET;  // 使用边沿触发模式， 保证每次只触发一个
    requestData *req = new requestData();
    req->setFd(listen_fd);
    epoll_add(epollfd, listen_fd, reinterpret_cast<void *>(req), event);  // TODO 这个地方需要把req指针转换为void*类型吗？
    while (true)
    {
        int nready = epoll_waits(epollfd, events, MAXEVENTS, -1);
        if(nready == 0)
        {
            continue;  // 什么时间会返回0ready? 达到timeout时间还是没有任何fd ready
        }
        else if (nready < 0)
        {
            break;
        }
        handle_events(epollfd, listen_fd, events, nready, PATH, threadpool);
        handle_expired_event();
    }
    return 0;
}