#include <sys/socket>
#include <netinet/in.h>

int main(int argc, char **agrs)
{
    int listenfd, connfd, sockfd;
    int client[FD_SETSIZE];
    fd_set rset, allset;
    socklen_t clilen;
    sockaddr_t cliaddr, servaddr;

    if (listenfd = socket(AF_INET, SOCKET_STREAM, 0) < 0) {
        err_sys("socket creation error.");
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_port = htons(1993);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_sin_family = AF_INET;

    if (bind(listenfd, reinterpret_cast<sockaddr*>(servaddr), sizeof(sockaddr) < 0) {
        sys_err("socket bind error.")
    }
    
    if (listen(listenfd, 1024) < 0) {
        sys_err("socket listen error.");
    }
    int maxfd = listenfd;
    int maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    for (;;)
    {
        
    }

}