/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-18 10:08:41
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 15:54:48
 */
#include "Client.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

const char* IPADDRESS = "127.0.0.1";
const int SERV_PORT = 8421;

Client::Client(EventLoop *loop, int threadNum, int port)
:   loop_(loop),
    threadNum_(threadNum),
    eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
    started_(false),
    clientChannel_(new Channel(loop_)),
    port_(port),
    clientFd_(socket_connect(const_cast<char*>(IPADDRESS), SERV_PORT))
    // connectionCallback_(defaultConnectionCallback),
    // messageCallback_(defaultMessageCallback)
{
    clientChannel_->setFd(clientFd_);
    handle_for_sigpipe();
    if (setSocketNonBlocking(clientFd_) < 0)
    {
        perror("set socket non block failed");
        abort();
    }
}

Client::~Client()
{
    if (clientFd_)
    {
        close(clientFd_);
    }
}

void Client::start()
{
    eventLoopThreadPool_->start();
    //acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    clientChannel_->setEvents(EPOLLIN | EPOLLET);
    clientChannel_->setReadHandler(bind(&Client::messageCallback, this, _1));
    clientChannel_->setConnHandler(bind(&Client::handThisConn, this));
    loop_->addToPoller(clientChannel_, 0);
    connectionCallback(clientFd_);  // handle connection call back on start.
    started_ = true;
}