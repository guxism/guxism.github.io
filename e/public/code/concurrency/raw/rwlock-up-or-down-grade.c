
void *process(void *arg);

#include <pthread.h>
#define NUM_READERS 4
#define NUM_WRITERS 4
#define THRESHOLD 1
struct context
{
    pthread_rwlock_t lock;
    pthread_attr_t attr;
    pthread_t writers[NUM_WRITERS];
    pthread_t readers[NUM_READERS];
    int shared;
};

enum position
{
    reader,
    writer
};

struct wrapper
{
    enum position pos;
    int id;
    struct context *ctx;
};

#include <stdlib.h>
void run(struct context *ctx)
{
    int i;
    for (i = 0; i < NUM_WRITERS; ++i)
    {
        struct wrapper *w = calloc(1, sizeof(struct wrapper));
        w->ctx = ctx;
        w->pos = writer;
        w->id = i;
        pthread_create(&ctx->writers[i], &ctx->attr, process, (void *)w);
    }
    for (i = 0; i < NUM_READERS; ++i)
    {
        struct wrapper *w = calloc(1, sizeof(struct wrapper));
        w->ctx = ctx;
        w->pos = reader;
        w->id = i;
        pthread_create(&ctx->readers[i], &ctx->attr, process, (void *)w);
    }
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
void begin(struct wrapper *w)
{
    struct context *ctx = w->ctx;
    int id = w->id;
    int pos = w->pos;
    if (pos == reader)
    {
        pthread_rwlock_rdlock(&ctx->lock);
    }
    else if (pos == writer)
    {
        pthread_rwlock_wrlock(&ctx->lock);
    }
}

void end(struct wrapper *w)
{
    pthread_rwlock_unlock(&w->ctx->lock);
}

#include <limits.h>
void *process(void *arg)
{
    struct wrapper *w = (struct wrapper *)arg;
    for (;;)
    {
        begin(w);
        {
            if (reader == w->pos)
            {
                if (THRESHOLD == w->ctx->shared)
                {
                    w->ctx->shared = 0;
                    int newid = ((w->id + 1) * 10 + w->id) % INT_MAX;
                    printf("[upgrade] reader %d -> writer %d\n", w->id, newid);
                    w->id = newid;
                    w->pos = writer;
                    end(w);
                    sleep(1);
                    continue;
                }
                else
                {
                    printf("[reader %d]: content: %d\n", w->id, w->ctx->shared);
                }
            }
            else if (writer == w->pos)
            {
                if (THRESHOLD == w->ctx->shared)
                {
                    int newid = ((w->id + 1) * 10 + w->id) % INT_MAX;
                    printf("[downgrade] writer %d -> reader %d\n", w->id, newid);
                    w->id = newid;
                    w->pos = reader;
                    end(w);
                    sleep(1);
                    continue;
                }
                else
                {
                    w->ctx->shared += 1;
                    printf("[writer %d]: content: %d\n", w->id, w->ctx->shared);
                }
            }
        }
        end(w);
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
    pthread_rwlock_init(&ctx->lock, NULL);
}

void ctx_destroy(struct context *ctx)
{
    int i;
    for (i = 0; i < NUM_WRITERS; ++i)
        pthread_join(ctx->writers[i], NULL);
    for (i = 0; i < NUM_READERS; ++i)
        pthread_join(ctx->readers[i], NULL);
    pthread_attr_destroy(&ctx->attr);
    pthread_rwlock_destroy(&ctx->lock);
}

