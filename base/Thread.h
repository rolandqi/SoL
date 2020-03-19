/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 15:23:30
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-17 15:11:58
 */
#ifndef BASE_THREAD_H_
#define BASE_THREAD_H_

#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>

using namespace std;

namespace base {
class Thread
{
public:
    typedef std::function<void()> ThreadFunc;

    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    ~Thread();

    void start();
    int join();
    bool started() const {return started_;}
    pid_t tid() const {return tid_;}
    const std::string & name() const {return name_;}
    static int numCreated() {return numCreated_;}  // TODO 这个函数能写成const吗？

private:
    void setDefaultName();
    bool started_;
    bool joined_;
    pthread_t pthreadId_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    // CountDownLatch latch_;
    static int numCreated_;
};
}

#endif /* BASE_THREAD_H_ */
