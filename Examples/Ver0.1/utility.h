#ifndef UTILITY_H
#define UTILITY_H

#include <errno.h>
#include <stdint.h>  // for uint_t

typedef std::uint8_t            UINT_8, *PUINT_8;  //sstream
typedef std::uint16_t           UINT_16, *PUINT_16;
typedef std::uint32_t           UINT_32, *PUINT_32;
typedef std::uint64_t           UINT_64, *PUINT_64;

void handle_for_sigpipe();
int setSocketNonBlocking(int fd);

# endif