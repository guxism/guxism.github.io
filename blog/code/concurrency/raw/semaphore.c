
void *inc(void *arg);
void *dec(void *);
void *watch(void *);

#include <pthread.h>
#include <semaphore.h>
struct context
{
    sem_t guard;
    sem_t ready;
    int shared;
    char signaller;
    pthread_attr_t attr;
    pthread_t t1, t2, t3;
};

void run(struct context *ctx)
{
    pthread_create(&ctx->t1, &ctx->attr, inc, (void *)ctx);
    pthread_create(&ctx->t2, &ctx->attr, dec, (void *)ctx);
    pthread_create(&ctx->t3, &ctx->attr, watch, (void *)ctx);
}

#include <strings.h>
#include <limits.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    sem_init(&ctx->guard, 0, 1);
    sem_init(&ctx->ready, 0, 1);
}

#include <stdio.h>
void ctx_destroy(struct context *ctx)
{
    pthread_join(ctx->t1, NULL);
    pthread_join(ctx->t2, NULL);
    pthread_join(ctx->t3, NULL);
    pthread_attr_destroy(&ctx->attr);
    sem_destroy(&ctx->guard);
    sem_destroy(&ctx->ready);
    printf("ctx->shared: %d\n", ctx->shared);
}

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
void *inc(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (;;)
    {
        sem_wait(&ctx->guard);
        int read = ctx->shared;
        sleep(0.5);
        int write = read + 1;
        ctx->shared = write;
        if (ctx->shared == 0)
        {
            printf("inc signal\n");
            ctx->signaller = 'A';
            sem_post(&ctx->ready);
        }

        sem_post(&ctx->guard);
    }
    pthread_exit((void *)0);
}

void *dec(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (;;)
    {
        sem_wait(&ctx->guard);
        int read = ctx->shared;
        sleep(0.5);
        int write = read - 1;
        ctx->shared = write;
        if (ctx->shared == 0)
        {
            printf("dec signal\n");
            ctx->signaller = 'B';
            sem_post(&ctx->ready);
        }
        sem_post(&ctx->guard);
    }
    pthread_exit((void *)0);
}

void *watch(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (;;)
    {
        sem_wait(&ctx->guard);
        while (ctx->shared != 0)
        {
            printf("Poll failed, enter waiting state\n");
            sem_post(&ctx->guard);
            sem_wait(&ctx->ready);
            printf("%c me excitat!\n", ctx->signaller);
            printf("好耶!\n");
            continue;
        }
        printf("好耶!\n");
        sem_post(&ctx->guard);
    }
    pthread_exit((void *)0);
}


