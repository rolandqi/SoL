#include <poll.h>
#include <sys/socket>
#include <netinet/in.h>
#include	<limits.h>		/* for OPEN_MAX */

/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define	LISTENQ		1024	/* 2nd argument to listen() */

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define	MAXSOCKADDR  128	/* max socket address structure size */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */


int main(int argc, char **args)
{
    sockaddr_t servaddr, cliaddr;
    pollfd client[OPEN_MAX];
    char buf[MAXLINE];
    int connfd = -1;
    int maxi = 0;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(1993);
    bind(listenfd, reinterpret_cast<sockaddr*>(&servaddr), sizeof(servaddr));
    listen(listenfd, LISTENQ);
    client[0].fd = listenfd;
    client[0].event = POLLRDNORM;
    for (int i = 1; i < OPEN_MAX; i++)
    {
        client[i].fd = -1;
    }
    maxi = 0;

    for (;;)
    {
        int nready = poll(clent, maxi + 1; INFTIM);
        if (client[0].revents & POLLRDNORM)
        {
            int = clilen = sizeof(cliaddr);
            connfd = accept(listenfd, reinterpret_cast<sockaddr*)(&cliaddr), &clilen);
            for (int i = 1; i < OPEN_MAX; i++)
            {
                if (client[i].fd == -1)
                {
                    client[i].fd = connfd;
                    client[i].evnet = POLLRDNORM;
                    if(i > maxi)
                    {
                        maxi = i;
                    }
                    break;
                }
            }
            if (--nready <= 0)
            {
                continue;
            }
        }

        for (int i = 0; i < maxi; i++)
        {
            if (int sockfd = client[i].fd < 0)
            {
                continue;
            }
            if (client[i].revent & (POLLRDNORM | POLLERR))
            {
                if ((int n = read(sockfd, buf, MAXLIEN)) < 0)
                {
                    if (errno == ECONNRESET)
                    {
                        printf("connetion aborted.")
                        close(sockfd);
                        client[i] = -1;
                    }
                }
                else if (n == 0)
                {
                    printf("connection closed by client.")
                    close(sockfd);
                    cilent[i] = -1;
                }
                else
                {
                    write(sockfd, buf, n);
                }
                if (--nready <= 0)
                {
                    continue;
                }
            }
        }
    }
}