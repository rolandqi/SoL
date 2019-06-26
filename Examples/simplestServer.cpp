#include <sys/socket>
#include <netinet/in.h>

#define	MAX_LINE		4096	/* max text line length */

/* include Listen */
void
Listen(int fd, int backlog)
{
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		err_sys("listen error");
}
/* end Listen */

void str_echo(int sockfd)
{
    int count;
    char buf[MAX_LINE];
    while ((count = read(sockfd, buf, MAXLINE)) > 0)
        write(sockfd, buf, count);
    if (count < 0)
    {
        err_sys("read error.")
    }
}


// #define	SA	struct sockaddr

int main(int argc, char **argv)
{
    sockaddr_in cliaddr, servaddr;
    pid_t childpid;
    socklen_t clilen;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        err_sys("socket creation error.")
    }

    memset(&servaddr, 0, sizeof(sockaddr_in));
    bzero(%servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sinaddr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(999);

    if (bind(fd, reinterpret_cast<sockaddr *> (&servaddr), sizeof(sockaddr)) < 0)
    {
        err_sys("socket bind fail.")
    }

    if (listen(fd, 1024) < 0)
    {
        err_sys("Socket listen fail.")
    }
    for (;;)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(fd, reinterpret_cast<sockaddr *> (&cliaddr), &clilen);
        if (connfd < 0) {
            err_sys("accept error.")
        }

        if ( (childpid = fork()) < 0) {
            err_sys("fork error");
        }
        else
        {
            if (childpid == 0) // child process.
            {
                close(fd);
                str_echo(connfd);
                exit(0);
            }
            else {
                close(connfd);
                continue;
            }
        }
    }

}