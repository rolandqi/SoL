# README

- [README](#readme)
  - [Introduction](#introduction)
  - [简单HTTP项目](#%e7%ae%80%e5%8d%95http%e9%a1%b9%e7%9b%ae)
    - [实现的功能](#%e5%ae%9e%e7%8e%b0%e7%9a%84%e5%8a%9f%e8%83%bd)
    - [遇到的困难](#%e9%81%87%e5%88%b0%e7%9a%84%e5%9b%b0%e9%9a%be)

---

## Introduction

Welcome to SoL(Server on Linux). I init this repo to practice net programming on Linux.

1. 我本着练习C++编程能力以及熟悉网络网络编程的目的开始本项目，刚开始只是想实现一个简单的HTTP server， 可以解析get、head请求， 可以处理静态资源，可以支持HTTP长连接。
   - 已经完成
2. 随着研究的深入，我慢慢了解到了高性能服务器的优点，在尝试着阅读过一些源码（比如：muduo, Nginx, asio）,我试图仿照他们编写一个属于我自己的服务器，主要参考了muduo源码。
   - 正在进行
   - 目的：
     1. 使用智能指针（shared_ptr和weak_ptr）进行对象生命周期的管理
     2. 设计自带缓冲区的IO
     3. 实现异步日志库
     4. 使用eventfd， timefd实现现成的异步唤醒和定时器的功能
     5. 使用RAII包装文件描述符

## 简单HTTP项目

### 实现的功能

- 项目位于Examples\Ver0.1下
- 实现了如下功能：
  1. 使用epoll边沿触发的io mutiplexing 技术, 使用非阻塞IO, 使用Reactor 模式。
      1. 使用边沿触发的原因：
  2. 使用多线程技术，创建了线程池处理响应
  3. 创建了一个任务队列，使用条件变量通知线程池处理任务
  4. 基于小跟堆的定时器实现了超时线程的关闭
  5. 解析了HTTP的get、post请求
- 锁的应用：
  1. mutex lock
  2. condition variables
- 进程使用：
  1. 主线程负责等待epoll中的事件，并把到来的事件放到任务队列中，在epoll_wait之后会结束超时的请求。
  2. 维护了一个数目固定的线程池，线程池在空闲时阻塞在条件变量的等待中，只有在新任务来到才被唤醒，执行IO操作和计算任务。
  
### 遇到的困难

1. reactor模式和平常接触的业务逻辑不同（工厂模式+基本的posix编程）
2. 多线程编程的竞态分析，我日常接触到的代码以单线程为主，就算遇到了多线程/多进程，也总是以最简单的semephore+共享内存解决问题。通过这次的学习，我学会了分析代码是否是线程安全的，在多线程编程中应该使用怎样的同步原语可以解决竞态问题。
