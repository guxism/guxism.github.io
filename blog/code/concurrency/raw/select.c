

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int set_nonblocking(int fd)
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_socket(int port)
{
    int listener_sockfd;
    struct sockaddr_in addr;

    listener_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listener_sockfd < 0)
    {
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    int optval = 1;
    setsockopt(listener_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    if (set_nonblocking(listener_sockfd) == -1)
    {
        return -1;
    }

    if (bind(listener_sockfd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        return -1;
    }

    if (listen(listener_sockfd, 5))
    {
        return -1;
    }

    return listener_sockfd;
}

#define BUFSIZE 128
int main(int argc, char **argv)
{
    int port = atoi(argv[1]);
    int listener_sockfd;
    if ((listener_sockfd = create_socket(port)) == -1)
    {
        return -1;
    }

    set_nonblocking(STDIN_FILENO);
    set_nonblocking(STDOUT_FILENO);
    set_nonblocking(STDERR_FILENO);

    char buf[BUFSIZE];

    struct timeval tv;
    int nready;

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    fd_set rfds_saved;
    fd_set wfds_saved;
    FD_ZERO(&rfds_saved);
    FD_ZERO(&wfds_saved);
    FD_SET(listener_sockfd, &rfds_saved);
    FD_SET(STDIN_FILENO, &rfds_saved);
    int fd_max = listener_sockfd;

    while (1)
    {
        fd_set rfds = rfds_saved;
        fd_set wfds = wfds_saved;
        nready = select(fd_max + 1, &rfds, &wfds, NULL, &tv);
        if (nready == -1)
        {
            perror("select()");
        }
        for (int fd = 0; fd < (fd_max + 1) && nready > 0; ++fd)
        {
            if (FD_ISSET(fd, &rfds))
            {
                --nready;
                if (fd == listener_sockfd)
                {
                    struct sockaddr_in c_addr;
                    socklen_t client_len = sizeof(c_addr);
                    int client =
                        accept(fd, (struct sockaddr *)&c_addr, &client_len);
                    set_nonblocking(client);

                    printf("New connection from %s:%i\n",
                           inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
                    FD_SET(client, &rfds_saved);
                    fd_max = client;
                }
                else if (fd == STDIN_FILENO)
                {
                    --nready;
                    bzero(buf, sizeof(buf));
                    int n = read(STDIN_FILENO, buf, 1);
                    if (n == 0)
                    {
                        FD_SET(STDOUT_FILENO, &wfds_saved);
                    }
                    else if (n < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                        }
                        else
                        {
                            perror("read");
                        }
                    }
                    else
                    {
                        printf("From STDIN: %s\n", buf);
                        if (buf[0] = '\n')
                        {
                            FD_SET(STDOUT_FILENO, &wfds_saved);
                        }
                    }
                }
                else
                {
                    --nready;
                    bzero(buf, sizeof(buf));
                    int n = recv(fd, buf, 1, 0);
                    if (n == 0)
                    {
                        FD_CLR(fd, &rfds_saved);
                    }
                    else if (n < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                        }
                        else
                        {
                            perror("recv");
                        }
                    }
                    else
                    {
                        printf("From socket [%d]: %s\n", fd, buf);
                        if (buf[0] = '\n')
                        {
                            FD_SET(fd, &wfds_saved);
                        }
                    }
                }
            }
            else if (FD_ISSET(fd, &wfds))
            {
                if (fd == STDOUT_FILENO)
                {
                    --nready;
                    bzero(buf, sizeof(buf));
                    sprintf(buf, "Hail STDIN!\n");
                    write(fd, buf, BUFSIZE);
                    FD_CLR(fd, &wfds_saved);
                }
                else
                {
                    --nready;
                    bzero(buf, sizeof(buf));
                    sprintf(buf, "Salute! Socket %d, your message is comfirmed\n", fd);
                    send(fd, buf, BUFSIZE, 0);
                    FD_CLR(fd, &wfds_saved);
                }
            }
        }
    }
}



void handler(int unused) { (void)unused; }

int main_thread()
{
    fd_set rfds;
    struct timeval tv;
    int retval;
    char buf[128];
    FD_ZERO(&rfds);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    FD_SET(0, &rfds);
    retval = select(1, &rfds, NULL, NULL, &tv);
    if (retval == -1)
    {
        if (errno == EINTR)
            printf("select() was interrupted\n");
        else
            perror("select()");
    }
    else if (retval)
    {
        bzero(buf, 128);
        read(0, buf, 128);
        printf("You say: %s\n", buf);
    }
}

