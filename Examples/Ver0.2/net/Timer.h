/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-17 09:52:02
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 11:46:44
 */
#ifndef NET_TIMER_H_
#define NET_TIMER_H_

#include "../base/Mutex.h"
#include "Channel.h"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>

class TimerNode
{
public:
    TimerNode(std::shared_ptr<Channel> requestData, int timeout);
    ~TimerNode();
    TimerNode(TimerNode &tn);
    void update(int timeout);
    bool isValid();
    void clearReq();
    void setDeleted() { deleted_ = true; }
    bool isDeleted() const { return deleted_; }
    size_t getExpTime() const { return expiredTime_; }

private:
    bool deleted_;
    size_t expiredTime_;
    std::shared_ptr<Channel> request_data;
};

struct TimerCmp
{
    bool operator()(std::shared_ptr<TimerNode> &a, std::shared_ptr<TimerNode> &b) const
    {
        return a->getExpTime() > b->getExpTime();
    }
};

class TimerManager
{
public:
    TimerManager();
    ~TimerManager();
    void addTimer(std::shared_ptr<Channel> requestData, int timeout);
    void handleExpiredEvent();

private:
    typedef std::shared_ptr<TimerNode> SPTimerNode;
    std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp> timerNodeQueue;
    base::MutexLock lock_;
};



#endif /* NET_TIMER_H_ */
