/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:22:50
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 11:34:06
 */
#ifndef NET_EPOLL_H_
#define NET_EPOLL_H_

#include <vector>
#include <unordered_map>
#include "Channel.h"

using namespace std;

class Epoll {
public:
    typedef std::shared_ptr<Channel> SP_Channel;
    Epoll();
    ~Epoll();
    void epoll_add(SP_Channel request, int timeout);
    void epoll_mod(SP_Channel request, int timeout);
    void epoll_del(SP_Channel request);
    void poll(int timeoutMs, vector<SP_Channel>* activeChannels);
    std::vector<SP_Channel> getEventsRequest(int events_num);
    void add_timer(SP_Channel request_data, int timeout);
    int getEpollFd()
    {
        return epollFd_;
    };
    void handleExpired();
private:
    static const int MAXFDS = 100000;
    int epollFd_;
    std::vector<epoll_event> events_;
    std::shared_ptr<Channel> fd2chan_[MAXFDS];
};

#endif /* NET_EPOLL_H_ */