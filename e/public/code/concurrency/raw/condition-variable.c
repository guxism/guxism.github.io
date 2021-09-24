
void *inc(void *arg);
void *dec(void *);
void *watch(void *);

#include <pthread.h>
struct context
{
    pthread_cond_t cv;
    int shared;
    pthread_mutex_t mutex;
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
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&ctx->mutex, NULL);
    pthread_cond_init(&ctx->cv, NULL);
}

#include <stdio.h>
void ctx_destroy(struct context *ctx)
{
    pthread_join(ctx->t1, NULL);
    pthread_join(ctx->t2, NULL);
    pthread_join(ctx->t3, NULL);
    pthread_attr_destroy(&ctx->attr);
    pthread_mutex_destroy(&ctx->mutex);
    pthread_cond_destroy(&ctx->cv);
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
    pthread_mutex_lock(&ctx->mutex);
    int read = ctx->shared;
    printf("inc: read %d\n", ctx->shared);
    sleep(0.5);
    int write = read + 1;
    ctx->shared = write;
    printf("inc: write %d\n", ctx->shared);
    if (ctx->shared == 0)
    {
        printf("inc signal\n");
        ctx->signaller = 'A';
        pthread_cond_signal(&ctx->cv);
    }

    pthread_mutex_unlock(&ctx->mutex);
    pthread_exit((void *)0);
}

void *dec(void *arg)
{
    struct context *ctx = (struct context *)arg;
    pthread_mutex_lock(&ctx->mutex);
    int read = ctx->shared;
    printf("dec: read %d\n", ctx->shared);
    sleep(0.5);
    int write = read - 1;
    ctx->shared = write;
    printf("dec: write %d\n", ctx->shared);
    if (ctx->shared == 0)
    {
        printf("dec signal\n");
        ctx->signaller = 'B';
        pthread_cond_signal(&ctx->cv);
    }

    pthread_mutex_unlock(&ctx->mutex);
    pthread_exit((void *)0);
}


void *watch(void *arg)
{
    struct context *ctx = (struct context *)arg;
    pthread_mutex_lock(&ctx->mutex);
    while (ctx->shared != 0)
    {
        printf("Poll failed, enter waiting state\n");
        pthread_cond_wait(&ctx->cv, &ctx->mutex);
        printf("%c me excitat!\n", ctx->signaller);
    }
    printf("好耶!\n");
    pthread_mutex_unlock(&ctx->mutex);
    pthread_exit((void *)0);
}



