/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-18 10:08:51
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 15:07:56
 */
#ifndef NET_CLIENT_H_
#define NET_CLIENT_H_


#include "EventLoopThreadPool.h"
#include <sys/time.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class Client
{
public:
    typedef std::function<void()> CallBack;
    typedef function<void (const int&)> MessageCallback;
    typedef function<void (const struct sockaddr_in&)> WriteCompleteCallback;
    typedef function<void (const struct sockaddr_in&)> ConnectionCallback;

    Client(EventLoop *loop, int threadNum, int port);
    ~Client();
    EventLoop* getLoop() const { return loop_; }
    void start();
    void handThisConn() { loop_->updatePoller(clientChannel_); }
    void setConnectionCallback(const MessageCallback& cb)  // Anchor：在调用Client的外部文件里面记得设置callback
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const CallBack& cb)
    { writeCompleteCallback_ = cb; }
    void connectionCallback(const int& fd)
    {
        if (connectionCallback_)
        {
            connectionCallback_(fd);
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
    std::shared_ptr<Channel> clientChannel_;
    int port_;
    int clientFd_;
    static const int MAXFDS = 100000;
    MessageCallback connectionCallback_; // 连接建立和关闭时的callback
    MessageCallback messageCallback_; // 消息到来时的callback
    CallBack writeCompleteCallback_; // 消息写入对方缓冲区时的callback
};

#endif /* NET_CLIENT_H_ */
