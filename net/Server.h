/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-17 13:40:09
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 11:45:57
 */
#ifndef NET_SERVER_H_
#define NET_SERVER_H_

#include "EventLoopThreadPool.h"
#include <sys/time.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class Server
{
public:
    typedef std::function<void()> CallBack;
    typedef function<void (const int&)> MessageCallback;
    typedef function<void (const struct sockaddr_in&)> WriteCompleteCallback;
    typedef function<void (const struct sockaddr_in&)> ConnectionCallback;

    Server(EventLoop *loop, int threadNum, int port);
    ~Server();
    EventLoop* getLoop() const { return loop_; }
    void start();
    void handNewConn();
    void handThisConn() { loop_->updatePoller(acceptChannel_); }
    void setConnectionCallback(const ConnectionCallback& cb)  // Anchor：在调用server的外部文件里面记得设置callback
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const CallBack& cb)
    { writeCompleteCallback_ = cb; }
    void connectionCallback(const struct sockaddr_in& request)
    {
        if (connectionCallback_)
        {
            connectionCallback_(request);
        }
    }
    void messageCallback(const int& fd)
    {
        if (messageCallback_)
        {
            messageCallback_(fd);
        }
    }
    void writeCompleteCallback()
    {
        if (writeCompleteCallback_)
        {
            writeCompleteCallback_();
        }
    }

private:
    EventLoop *loop_;
    int threadNum_;
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
    bool started_;
    std::shared_ptr<Channel> acceptChannel_;
    int port_;
    int listenFd_;
    static const int MAXFDS = 100000;
    ConnectionCallback connectionCallback_; // 连接建立和关闭时的callback
    MessageCallback messageCallback_; // 消息到来时的callback
    CallBack writeCompleteCallback_; // 消息写入对方缓冲区时的callback
};

void defaultConnectionCallback();
void defaultMessageCallback();

#endif /* NET_SERVER_H_ */
