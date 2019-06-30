/*
 * main.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: kaiqi
 */

#include "utility.h"
#include "epoll.h"
#include <pthread.h>

using namespace std;

#define MAX_EVENTS 10
#define PORT 3389
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    // handle for sigpipe
    int epollfd = epoll_init();
    if (epollfd < 0)
    {
        return 1;
    }

    
    
}