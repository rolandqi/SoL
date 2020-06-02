# README

- [README](#readme)
  - [简介](#简介)
  - [项目进展](#项目进展)
    - [Ver1.0实现的功能](#ver10实现的功能)
    - [正式版本](#正式版本)
  - [延伸](#延伸)

---

## 简介

Welcome to SoL(Server on Linux). I init this repo to practice net programming on Linux.

1. 我本着练习C++编程能力以及熟悉网络网络编程的目的开始本项目，刚开始只是想实现一个简单的HTTP server， 可以解析get、head请求， 可以处理静态资源，可以支持HTTP长连接。
   - 目标代码在`Example/Ver0.1`(已经完成)
2. 随着研究的深入，我慢慢了解到了高性能服务器的优点，在尝试着阅读过一些源码（比如：muduo, Nginx, asio）,我试图仿照他们编写一个属于我自己的服务器，主要参考了muduo源码。
   - 第一步：使用基本muduo的思想，实现一个较高性能的服务器和客户端
     1. 包含muduo基本的思想，包括Eventlooppool, queueinloop（包括eventfd和timefd）, RAII, 锁等。
     2. 代码实现在`Example/Ver0.2`(已经完成)
   - 第二步：
     1. 设计自带缓冲区的IO
     2. 实现异步日志库
     3. 性能测试
     4. 与muduo进行比较

## 项目进展

### Ver1.0实现的功能

- 项目位于`Examples\Ver0.1`下
- 实现了如下功能：
  1. 使用epoll边沿触发的io mutiplexing技术, 使用非阻塞IO, 使用Reactor模式。
      1. 使用边沿触发的原因：
  2. 使用多线程技术，创建了线程池处理响应。
  3. 创建了一个任务队列，使用条件变量通知线程池处理任务。
  4. 基于小跟堆的定时器实现了超时线程的关闭。
  5. 解析了HTTP的get、post请求，可以处理静态资源，可以支持HTTP长连接。
- 锁的应用：
  1. mutex lock
  2. condition variables
- 进程使用：
  1. 主线程负责等待epoll中的事件，并把到来的事件放到任务队列中，在`epoll_wait`之后会结束超时的请求。
  2. 维护了一个数目固定的线程池，线程池在空闲时阻塞在条件变量的等待中，只有在新任务来到才被唤醒，执行IO操作和计算任务。

### 正式版本

- 与上一版相比，实现了如下功能：
  1. eventloop每个线程只存在一个，其生命周期和其所属的线程一样长。
  2. channel对象自始自终只负责一个文件描述符fd的事件分发。
  3. Epoll对IO mutiplexing进行封装，poller类并不直接持有Channel类，而是在获得当前活动的Channel并返回。
  4. 上述3个类构成了Reactor的核心内容。
  5. Server/Client类实现了一个阻塞的网络库。完成了write/read等操作。
  6. 日志库
- 锁的应用：
  1. RAII mutex lock
  2. condition variables
- pingpong 测试结果与muduo比较，性能较为接近。

## 延伸

- mongoDB官方的mongo CXX driver为同步接口，如果线程，特别是事件驱动的reactor，直接使用同步接口，有可能会造成其他线程的死锁。
- 为解决这个问题，我用本网络库搭建了一个mongoDB异步中间件，它把mongo CRUD操作封装为protobuf请求，收到请求之后派发到线程池中对mongoDB进行操作，完成之后返回结果。
