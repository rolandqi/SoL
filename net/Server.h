/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 13:37:19
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-16 13:37:19
 */
/*
 * Server.h
 *
 *  Created on: Aug 21, 2019
 *      Author: kaiqi
 */

#ifndef NET_SERVER_H_
#define NET_SERVER_H_

#include "EventLoop.h"
#include "Channel.h"
#include "EventLoopThreadPool.h"
#include <memory>


class Server
{
public:
    Server(EventLoop *loop, int threadNum, int port);
    ~Server() { }
    EventLoop* getLoop() const { return loop_; }
    void start();
    void handNewConn();
    void handThisConn() { loop_->updatePoller(acceptChannel_); }

private:
    EventLoop *loop_;
    int threadNum_;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    bool started_;
    std::shared_ptr<Channel> acceptChannel_;
    int port_;
    int listenFd_;
    static const int MAXFDS = 100000;
};



#endif /* NET_SERVER_H_ */
