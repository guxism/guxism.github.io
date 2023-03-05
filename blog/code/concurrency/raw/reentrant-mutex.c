
void *rec_descent(void *arg);
void *rec_ascent(void *arg);

#include <pthread.h>
struct context
{
    pthread_attr_t attr;
    int shared;
    pthread_t t1, t2;
    pthread_mutex_t mutex;
    pthread_mutexattr_t mta;
};

#include <unistd.h>
void run(struct context *ctx)
{
    pthread_create(&ctx->t2, &ctx->attr, rec_ascent, (void *)ctx);
    pthread_create(&ctx->t1, &ctx->attr, rec_descent, (void *)ctx);
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
void *rec_descent(void *arg)
{
    struct context *ctx = (struct context *)arg;
    pthread_mutex_lock(&ctx->mutex);
    printf("descent: lock\n");

    if (ctx->shared != 0)
    {
        ctx->shared -= 1;
        rec_descent(arg);
    }
    printf("descent: unlock\n");
    pthread_mutex_unlock(&ctx->mutex);
    return NULL;
}

void *rec_ascent(void *arg)
{
    struct context *ctx = (struct context *)arg;
    pthread_mutex_lock(&ctx->mutex);
    printf("ascent lock\n");

    if (ctx->shared != 10)
    {
        ctx->shared += 1;
        rec_ascent(arg);
    }
    printf("ascent: unlock\n");
    pthread_mutex_unlock(&ctx->mutex);
    return NULL;
}

#include <strings.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutexattr_init(&ctx->mta);
    pthread_mutexattr_settype(&ctx->mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&ctx->mutex, &ctx->mta);
}

void ctx_destroy(struct context *ctx)
{
    pthread_join(ctx->t1, NULL);
    pthread_join(ctx->t2, NULL);
    pthread_attr_destroy(&ctx->attr);
    pthread_mutexattr_destroy(&ctx->mta);
    pthread_mutex_destroy(&ctx->mutex);
    printf("ctx->shared: %d\n", ctx->shared);
}


