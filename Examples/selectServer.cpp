#include <sys/socket>
#include <netinet/in.h>
#include <sys/select.h>

int main(int argc, char **agrs)
{
    int listenfd, connfd, sockfd;
    int client[FD_SETSIZE];
    fd_set rset, allset;
    char buf[MAXLINE];
    
    sockaddr_t cliaddr, servaddr;
    socklen_t clilen = sizeof(cliaddr);
    
    if (listenfd = socket(AF_INET, SOCKET_STREAM, 0) < 0) {
        err_sys("socket creation error.");
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_port = htons(1993);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_sin_family = AF_INET;

    if (bind(listenfd, reinterpret_cast<sockaddr*>(servaddr), sizeof(sockaddr)) < 0) {
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
        rset = allset;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0)
        {
            printf("select error")
            return 1;
        }
        if (FD_ISSET(listenfd, rset))
        {
            connfd = accept(listenfd, reinterpret_cast<sockaddr*>&cliaddr, &clilen);
            if (connfd < 0) {
                printf("accept error.");
                return 1;
            }
            int i = 0;
            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = connfd;
                    if (i > maxi)
                    {
                        maxi = i;
                    }
                    break;
                }
            }
            if (i == FD_SETSIZE)
            {
                printf("too many clients.");
                return 1;
            }
            FD_SET(connfd, &allset);
            if (connfd > maxfd)
            {
                maxfd = connfd;
            }
            if (--nready <= 0) {  // 如果nready去除listenfd 之后仍大于1,说明有另外注册的connfd已经ready for read了
                continue;
            }
        }

        for (int i = 0; i <= maxi; i++) {
            if ((sockfd =client[i]) < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &reset))
            {
                if ((n = read(sockfd, buf, MAXLINE)) <= 0)
                {
                    if (n == 0)
                    {
                        close(sockfd);
                        client[i] = -1;
                        FD_CLR(sockfd, buf, MAXLINE);
                    }
                    else
                    {
                        printf("read error");
                        return 1;
                    }
                }
                else
                {
                    write(sockfd, buf, n);
                }
            }

            if (--nready <= 0)
                break;				/* no more readable descriptors */
        }
    }

}