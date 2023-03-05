
#define NUM 8
#define ROUND 3

int fib(int i)
{
    if (i == 0 || i == 1)
        return i;
    else
        return fib(i - 1) + fib(i - 2);
}

#include <time.h>
#include <pthread.h>
#include <stdlib.h>
struct context
{
    pthread_barrier_t barrier;
    pthread_attr_t attr;
    pthread_t t[NUM];
    clock_t time[NUM];
};

struct wrapper
{
    int id;
    struct context *ctx;
};

#include <stdio.h>
void *run(void *arg)
{
    struct wrapper *w = (struct wrapper *)arg;
    for (int i = 0; i < ROUND; ++i)
    {
        clock_t t = clock();
        printf("<%d> start\n", w->id);
        fib(40);
        printf("<%d> end\n", w->id);
        t = clock() - t;
        w->ctx->time[w->id] += t;
        pthread_barrier_wait(&w->ctx->barrier);
    }
    free(arg);
}

void race(struct context *ctx)
{
    for (int i = 0; i < NUM; ++i)
    {
        struct wrapper *w = calloc(1, sizeof(struct wrapper));
        w->ctx = ctx;
        w->id = i;
        pthread_create(&ctx->t[i], &ctx->attr, run, (void *)w);
    }
}

#include <strings.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    pthread_barrier_init(&ctx->barrier, NULL, NUM);
}

void ctx_destroy(struct context *ctx)
{
    for (int i = 0; i < NUM; ++i)
    {
        pthread_join(ctx->t[i], NULL);
    }
    pthread_attr_destroy(&ctx->attr);
    pthread_barrier_destroy(&ctx->barrier);
    for (int i = 0; i < NUM; ++i)
    {
        printf("[%d]: %f seconds\n", i, ((double)(ctx->time[i])) / CLOCKS_PER_SEC / ROUND);
    }
}

int main()
{
    struct context ctx;
    ctx_init(&ctx);
    race(&ctx);
    ctx_destroy(&ctx);
    pthread_exit(NULL);
}


