
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#define SENDBUF_SIZE 1024

void on_peer_connected(uv_stream_t *server_stream, int status);
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void on_client_closed(uv_handle_t *handle)
{
    uv_tcp_t *client = (uv_tcp_t *)handle;
    free(client);
}

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
void echo_write(uv_write_t *req, int status);

int main(int argc, const char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int portnum = 9000;
    if (argc >= 2)
    {
        portnum = atoi(argv[1]);
    }
    fprintf(stdout, "Serving on port %d\n", portnum);

    int rc;
    uv_tcp_t server_stream;
    if ((rc = uv_tcp_init(uv_default_loop(), &server_stream)) < 0)
    {
        fprintf(stderr, "uv_tcp_init failed: %s", uv_strerror(rc));
        exit(-1);
    }

    struct sockaddr_in server_address;
    if ((rc = uv_ip4_addr("0.0.0.0", portnum, &server_address)) < 0)
    {
        fprintf(stderr, "uv_ip4_addr failed: %s", uv_strerror(rc));
        exit(-1);
    }

    if ((rc = uv_tcp_bind(&server_stream,
                          (const struct sockaddr *)&server_address, 0)) < 0)
    {
        fprintf(stderr, "uv_tcp_bind failed: %s", uv_strerror(rc));
        exit(-1);
    }

    if ((rc = uv_listen((uv_stream_t *)&server_stream, 64, on_peer_connected)) <
        0)
    {
        fprintf(stderr, "uv_listen failed: %s", uv_strerror(rc));
        exit(-1);
    }

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    return uv_loop_close(uv_default_loop());
}

void on_peer_connected(uv_stream_t *server_stream, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "Peer connection error: %s\n", uv_strerror(status));
        return;
    }
    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    int rc;
    if ((rc = uv_tcp_init(uv_default_loop(), client)) < 0)
    {
        fprintf(stderr, "uv_tcp_init failed: %s", uv_strerror(rc));
    }


    if (uv_accept(server_stream, (uv_stream_t *)client) == 0)
    {
        struct sockaddr_storage peername;
        int namelen = sizeof(peername);
        if ((rc = uv_tcp_getpeername(client, (struct sockaddr *)&peername,
                                     &namelen)) < 0)
        {
            fprintf(stderr, "uv_tcp_getpeername failed: %s", uv_strerror(rc));
        }

        struct sockaddr_in *sa = (struct sockaddr_in *)&peername;
        char hostbuf[NI_MAXHOST];
        char portbuf[NI_MAXSERV];
        if (getnameinfo((struct sockaddr *)sa, namelen, hostbuf, NI_MAXHOST,
                        portbuf, NI_MAXSERV, 0) == 0)
        {
            printf("peer (%s, %s) connected\n", hostbuf, portbuf);
        }
        else
        {
            printf("peer (unknonwn) connected\n");
        }

        int rc;
        if ((rc = uv_read_start((uv_stream_t *)client, alloc_buffer,
                                echo_read)) < 0)
        {
            fprintf(stderr, "uv_read_start failed: %s", uv_strerror(rc));
        }
    }
    else
    {
        uv_close((uv_handle_t *)client, on_client_closed);
    }
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }
    if (nread < 0)
    {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)client, on_client_closed);
    }

    free(buf->base);
}

void echo_write(uv_write_t *req, int status)
{
    if (status)
    {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

