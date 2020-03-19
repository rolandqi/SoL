/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-17 09:52:09
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 11:48:56
 */
#include "Timer.h"
#include <sys/time.h>
#include <unistd.h>
#include <queue>


TimerNode::TimerNode(std::shared_ptr<Channel> requestData, int timeout)
:   deleted_(false),
    request_data(requestData)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    // 以毫秒计
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode()
{
    if (request_data)
    {
        // request_data->handleClose();  //TODO
    }
}

TimerNode::TimerNode(TimerNode &tn):
    request_data(tn.request_data)
{ }


void TimerNode::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::isValid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if (temp < expiredTime_)
        return true;
    else
    {
        this->setDeleted();
        return false;
    }
}

void TimerNode::clearReq()
{
    // request_data.reset(); //TODO
    this->setDeleted();
}


TimerManager::TimerManager()
{ }

TimerManager::~TimerManager()
{ }

void TimerManager::addTimer(std::shared_ptr<Channel> requestData, int timeout)
{
    SPTimerNode new_node(new TimerNode(requestData, timeout));
    timerNodeQueue.push(new_node);
    // requestData->linkTimer(new_node);  TODO link timer to Channel
}


// timeNode 可以在handleExpireEvent里面更新，也可以用set（红黑树）去做

void TimerManager::handleExpiredEvent()
{
    base::MutexLockGuard locker(lock_);
    while (!timerNodeQueue.empty())
    {
        SPTimerNode ptimer_now = timerNodeQueue.top();
        if (ptimer_now->isDeleted())
            timerNodeQueue.pop();
        else if (ptimer_now->isValid() == false)
            timerNodeQueue.pop();
        else
            break;
    }
}
