
void *read_fn(void *arg);
void *write_fn(void *arg);

#include <pthread.h>
#define NUM_READERS 4
#define NUM_WRITERS 4
struct context
{
    pthread_attr_t attr;
    pthread_mutex_t rlock;
    pthread_mutex_t wlock;
    int rnum;
    pthread_t writers[NUM_WRITERS];
    pthread_t readers[NUM_READERS];
    int shared;
};

void run(struct context *ctx)
{
    int i;
    for (i = 0; i < NUM_WRITERS; ++i)
        pthread_create(&ctx->writers[i], &ctx->attr, write_fn, (void *)ctx);
    for (i = 0; i < NUM_READERS; ++i)
        pthread_create(&ctx->readers[i], &ctx->attr, read_fn, (void *)ctx);
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

#include <stdio.h>
#include <unistd.h>
void begin_read(struct context *ctx)
{
    pthread_mutex_lock(&ctx->rlock);
    ++ctx->rnum;
    if (1 == ctx->rnum)
    {
        pthread_mutex_lock(&ctx->wlock);
    }
    pthread_mutex_unlock(&ctx->rlock);
}

void end_read(struct context *ctx)
{
    pthread_mutex_lock(&ctx->rlock);
    --ctx->rnum;
    if (0 == ctx->rnum)
    {
        pthread_mutex_unlock(&ctx->wlock);
    }
    pthread_mutex_unlock(&ctx->rlock);
}

void *read_fn(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (;;)
    {
        begin_read(ctx);
        {
            sleep(0.5);
            printf("[reader]: current readers number: %d, content: %d\n", ctx->rnum, ctx->shared);
        }
        end_read(ctx);
        sleep(0.1);
    }
    pthread_exit((void *)0);
}

void *write_fn(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (;;)
    {
        pthread_mutex_lock(&ctx->wlock);
        ctx->shared = ++ctx->shared % 10;
        sleep(2);
        printf("[writer]: content: %d\n", ctx->shared);
        pthread_mutex_unlock(&ctx->wlock);
        sleep(1);
    }
    pthread_exit((void *)0);
}

#include <strings.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&ctx->rlock, NULL);
    pthread_mutex_init(&ctx->wlock, NULL);
}

void ctx_destroy(struct context *ctx)
{
    int i;
    for (i = 0; i < NUM_WRITERS; ++i)
        pthread_join(ctx->writers[i], NULL);
    for (i = 0; i < NUM_READERS; ++i)
        pthread_join(ctx->readers[i], NULL);
    pthread_attr_destroy(&ctx->attr);
    pthread_mutex_destroy(&ctx->rlock);
    pthread_mutex_destroy(&ctx->wlock);
}

