/*
 * @Description: qikai's network library
 * @Author: qikai
 * @Date: 2019-10-16 14:42:32
 * @LastEditors: qikai
 * @LastEditTime: 2019-10-18 15:59:31
 */
#ifndef NET_UTIL_H_
#define NET_UTIL_H_

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <time.h>
#include <iostream>
#include "../base/logging.h"

ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, char *ptr, size_t n);
ssize_t writen(int fd, std::string &sbuff);
void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
void setSocketNoLinger(int fd);
void shutDownWR(int fd);
int socket_bind_listen(int port);
int socket_connect(char *addr, int port);


#endif /* NET_UTIL_H_ */
