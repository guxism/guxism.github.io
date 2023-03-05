


#include <ev.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>


int create_socket(int port)
{
    int sd;
    struct sockaddr_in addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if (sd < 0)
    {
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        return -1;
    }

    if (listen(sd, 5))
    {
        return -1;
    }

    return sd;
}

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

#include <stdlib.h>
int main(int argc, char **argv)
{
    int port = atoi(argv[1]);
    int sd;
    if ((sd = create_socket(port)) == -1)
    {
        return -1;
    }

    struct ev_io w_accept;
    struct ev_loop *loop = ev_default_loop(0);
    ev_io_init(&w_accept, accept_cb, sd, EV_READ);
    ev_io_start(loop, &w_accept);

    while (1)
    {
        ev_loop(loop, 0);
    }

    return 0;
}

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    struct sockaddr_in c_addr;
    socklen_t client_len = sizeof(c_addr);
    int c_sd;
    struct ev_io *w_client = (struct ev_io *)malloc(sizeof(struct ev_io));

    if (EV_ERROR & revents)
    {
        perror("got invalid event");
        return;
    }

    c_sd =
        accept(watcher->fd, (struct sockaddr *)&c_addr, &client_len);

    if (c_sd < 0)
    {
        return;
    }

    printf("Connected with client from IP: %s and port: %i\n",
           inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));

    ev_io_init(w_client, read_cb, c_sd, EV_READ);
    ev_io_start(loop, w_client);
}

#include <unistd.h>
#include <time.h>
#define BUFSIZE 128
void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    char buffer[BUFSIZE];
    ssize_t read;


    if (EV_ERROR & revents)
    {
        perror("got invalid event");
        return;
    }

    read = recv(watcher->fd, buffer, BUFSIZE, 0);

    if (read < 0)
    {
        perror("read error");
        return;
    }

    if (read == 0)
    {
        ev_io_stop(loop, watcher);
        free(watcher);
        return;
    }
    else
    {
        time_t seconds;
        seconds = time(NULL);
        printf("[%ld]: message received, processing...\n", seconds);
        sleep(3);
        printf("[%ld]: message processed \n", seconds);
    }

    send(watcher->fd, buffer, read, 0);
    bzero(buffer, read);
}

