/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-17 13:40:17
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 14:21:25
 */
#include "Server.h"

Server::Server(EventLoop *loop, int threadNum, int port)
:   loop_(loop),
    threadNum_(threadNum),
    eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
    started_(false),
    acceptChannel_(new Channel(loop_)),
    port_(port),
    listenFd_(socket_bind_listen(port_))
    // connectionCallback_(defaultConnectionCallback),
    // messageCallback_(defaultMessageCallback)
{
    acceptChannel_->setFd(listenFd_);
    handle_for_sigpipe();
    if (setSocketNonBlocking(listenFd_) < 0)
    {
        perror("set socket non block failed");
        abort();
    }
}

Server::~Server()
{
    if (listenFd_)
    {
        close(listenFd_);
    }
}

void Server::start()
{
    eventLoopThreadPool_->start();
    //acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
    acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
    loop_->addToPoller(acceptChannel_, 0);
    started_ = true;
}

void Server::handNewConn()
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        EventLoop *loop = eventLoopThreadPool_->getNextLoop();
        cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;
        // cout << "new connection" << endl;
        // cout << inet_ntoa(client_addr.sin_addr) << endl;
        // cout << ntohs(client_addr.sin_port) << endl;
        /*
        // TCP的保活机制默认是关闭的
        int optval = 0;
        socklen_t len_optval = 4;
        getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
        cout << "optval ==" << optval << endl;
        */
        // 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }
        // 设为非阻塞模式
        if (setSocketNonBlocking(accept_fd) < 0)
        {
            cout << "Set non block failed!"<<endl;
            //perror("Set non block failed!");
            return;
        }

        setSocketNodelay(accept_fd);
        //setSocketNoLinger(accept_fd);
        shared_ptr<Channel> newChannel = make_shared<Channel>(loop);
        newChannel->setFd(accept_fd);
        newChannel->setCliAddr(client_addr);
        newChannel->setEvents(EPOLLIN | EPOLLET);
        newChannel->setReadHandler(bind(&Server::messageCallback, this, _1));
        newChannel->setConnHandler(bind(&Server::connectionCallback, this, _1));
        loop->addToPoller(newChannel, 0);
        // 做一个dummy任务来wake worker reactor
        loop->queueInLoop(std::bind(&Channel::wakeFunctor, newChannel));
    }
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}

void defaultConnectionCallback(const struct sockaddr_in& request)
{
}

void defaultMessageCallback(const int& fd)
{
    cout<< "fd " << fd <<endl;
}