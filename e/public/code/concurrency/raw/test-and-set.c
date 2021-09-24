void *inc(void *arg);
void *dec(void *);

struct context;
void run(struct context *ctx);

#include <pthread.h>
struct context
{
    pthread_attr_t attr;
    int shared;
    pthread_t t1, t2;
    int mutex;
};

void lock__(int *mutex)
{
    while (1 == __sync_lock_test_and_set(mutex, 1))
        ;
}
void unlock__(int *mutex)
{
    *mutex = 0;
}

#include <stdio.h>
#include <unistd.h>
void *inc(void *arg)
{
    struct context *ctx = (struct context *)arg;
    lock__(&ctx->mutex);
    {
        int read = ctx->shared;
        printf("inc: read %d\n", ctx->shared);
        sleep(2);
        int write = read + 1;
        ctx->shared = write;
        printf("inc: write %d\n", ctx->shared);
    }
    unlock__(&ctx->mutex);
    pthread_exit((void *)0);
}

void *dec(void *arg)
{
    struct context *ctx = (struct context *)arg;
    lock__(&ctx->mutex);
    {
        int read = ctx->shared;
        printf("dec: read %d\n", ctx->shared);
        sleep(2);
        int write = read - 1;
        ctx->shared = write;
        printf("dec: write %d\n", ctx->shared);
    }
    unlock__(&ctx->mutex);
    pthread_exit((void *)0);
}

void run(struct context *ctx)
{
    pthread_create(&ctx->t1, &ctx->attr, inc, (void *)ctx);
    pthread_create(&ctx->t2, &ctx->attr, dec, (void *)ctx);
}

void ctx_init(struct context *ctx);
void ctx_destroy(struct context *ctx);

int main()
{
    struct context ctx;
    ctx_init(&ctx);
    run(&ctx);
    ctx_destroy(&ctx);
    pthread_exit(NULL);
}

#include <strings.h>
#include <limits.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    ctx->mutex = INT_MAX;
}

void ctx_destroy(struct context *ctx)
{
    pthread_join(ctx->t1, NULL);
    pthread_join(ctx->t2, NULL);
    pthread_attr_destroy(&ctx->attr);
    printf("ctx->shared: %d\n", ctx->shared);
}


