
void *read_fn(void *arg);
void *write_fn(void *arg);

#include <pthread.h>
#define NUM_READERS 6
#define NUM_WRITERS 2
struct context
{
    pthread_mutex_t g;
    pthread_attr_t attr;
    pthread_cond_t cond;
    int num_readers_active;
    int num_writers_waiting;
    int writer_active;
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
    pthread_mutex_lock(&ctx->g);
    while (ctx->num_writers_waiting > 0 || ctx->writer_active)
    {
        pthread_cond_wait(&ctx->cond, &ctx->g);
    }
    ++ctx->num_readers_active;
    pthread_mutex_unlock(&ctx->g);
}

void end_read(struct context *ctx)
{
    pthread_mutex_lock(&ctx->g);
    --ctx->num_readers_active;
    if (0 == ctx->num_readers_active)
    {
        pthread_cond_signal(&ctx->cond);
    }
    pthread_mutex_unlock(&ctx->g);
}

void *read_fn(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (;;)
    {
        begin_read(ctx);
        {
            sleep(0.5);
            printf("[reader]: current readers number: %d, content: %d\n", ctx->num_readers_active, ctx->shared);
        }
        end_read(ctx);
        sleep(1);
    }
    pthread_exit((void *)0);
}

void begin_write(struct context *ctx)
{
    pthread_mutex_lock(&ctx->g);
    ++ctx->num_writers_waiting;
    while (ctx->num_readers_active > 0 || ctx->writer_active)
    {
        pthread_cond_wait(&ctx->cond, &ctx->g);
    }
    --ctx->num_writers_waiting;
    ctx->writer_active = 1;
    pthread_mutex_unlock(&ctx->g);
}

void end_write(struct context *ctx)
{
    pthread_mutex_lock(&ctx->g);
    ctx->writer_active = 0;
    pthread_cond_signal(&ctx->cond);
    pthread_mutex_unlock(&ctx->g);
}

void *write_fn(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (;;)
    {
        begin_write(ctx);
        {
            ctx->shared = ++ctx->shared % 10;
            sleep(2);
            printf("[writer]: content: %d\n", ctx->shared);
        }
        end_write(ctx);
        sleep(10);
    }
    pthread_exit((void *)0);
}

#include <strings.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&ctx->g, NULL);
    pthread_cond_init(&ctx->cond, NULL);
}

void ctx_destroy(struct context *ctx)
{
    int i;
    for (i = 0; i < NUM_WRITERS; ++i)
        pthread_join(ctx->writers[i], NULL);
    for (i = 0; i < NUM_READERS; ++i)
        pthread_join(ctx->readers[i], NULL);
    pthread_attr_destroy(&ctx->attr);
    pthread_mutex_destroy(&ctx->g);
    pthread_cond_destroy(&ctx->cond);
}



