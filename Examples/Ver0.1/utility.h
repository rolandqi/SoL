#ifndef UTILITY_H
#define UTILITY_H

#include <errno.h>
#include <cstdint>  // for uint_t

typedef std::uint8_t            UINT_8, *PUINT_8;  //sstream
typedef std::uint16_t           UINT_16, *PUINT_16;
typedef std::uint32_t           UINT_32, *PUINT_32;
typedef std::uint64_t           UINT_64, *PUINT_64;

// 有请求出现但是读不到数据,可能是Request Aborted,
// 或者来自网络的数据没有达到等原因,
// 对这样的请求尝试超过一定的次数就抛弃
const int AGAIN_MAX_TIMES = 10;

void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
ssize_t writen(int sockfd, const void* buf, int length);
ssize_t readn(int sockfd, void* buf, int length);

# endif