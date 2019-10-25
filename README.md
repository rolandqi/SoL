<!--
 * @Description: qikai's programming project
 * @Author: qikai
 * @Date: 2019-10-16 13:37:19
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-25 17:25:01
 -->

# README

- [README](#readme)
  - [简介](#%e7%ae%80%e4%bb%8b)
  - [项目进展](#%e9%a1%b9%e7%9b%ae%e8%bf%9b%e5%b1%95)
    - [Ver1.0实现的功能](#ver10%e5%ae%9e%e7%8e%b0%e7%9a%84%e5%8a%9f%e8%83%bd)
    - [Ver0.2](#ver02)
    - [正式项目](#%e6%ad%a3%e5%bc%8f%e9%a1%b9%e7%9b%ae)
  - [遇到的困难](#%e9%81%87%e5%88%b0%e7%9a%84%e5%9b%b0%e9%9a%be)

---

## 简介

Welcome to SoL(Server on Linux). I init this repo to practice net programming on Linux.

1. 我本着练习C++编程能力以及熟悉网络网络编程的目的开始本项目，刚开始只是想实现一个简单的HTTP server， 可以解析get、head请求， 可以处理静态资源，可以支持HTTP长连接。
   - 目标代码在`Example/Ver0.1`(已经完成)
2. 随着研究的深入，我慢慢了解到了高性能服务器的优点，在尝试着阅读过一些源码（比如：muduo, Nginx, asio）,我试图仿照他们编写一个属于我自己的服务器，主要参考了muduo源码。
   - 第一步：使用基本muduo的思想，实现一个较高性能的服务器和客户端
     1. 包含muduo基本的思想，包括Eventlooppool, queueinloop（包括eventfd和timefd）, RAII, 锁等。
     2. 代码实现在`Example/Ver0.2`(已经完成)
   - 第二步（进行中）：
     1. 进一步完善代码
     2. 设计自带缓冲区的IO
     3. 实现异步日志库
     4. 性能测试
     5. 与libevent等主流库进行比较

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

### Ver0.2

- 项目位于`Examples\Ver0.2`下
- 与上一版相比，实现了如下功能：
  1. eventloop每个线程只存在一个，其生命周期和其所属的线程一样长。
  2. channel对象自始自终只负责一个文件描述符fd的事件分发。
  3. Epoll对IO mutiplexing进行封装，poller类并不直接持有Channel类，而是在获得当前活动的Channel并返回。
  4. 上述3个类构成了Reactor的核心内容。
  5. Server/Client类实现了一个阻塞的网络库。完成了write/read等操作。
- 锁的应用：
  1. RAII mutex lock
  2. condition variables

### 正式项目

## 遇到的困难

1. reactor模式和平常接触的业务逻辑不同（工厂模式+基本的posix编程）
2. 多线程编程的竞态分析，我日常接触到的代码以单线程为主，就算遇到了多线程/多进程，也总是以最简单的semephore+共享内存解决问题。通过这次的学习，我学会了分析代码是否是线程安全的，在多线程编程中应该使用怎样的同步原语可以解决竞态问题。
3. 学习muduo中的runInLoop和queueInLoop的设计方法，这两个方法主要用来执行用户的某个回调函数，queueInLoop是跨进程调用的精髓所在，具有极大的灵活性，我们只需要绑定好回调函数就可以了。
