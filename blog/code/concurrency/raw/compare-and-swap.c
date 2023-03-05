
struct node;
struct pointer_t
{
    struct node *ptr;
    unsigned int count;
};

union cas_wrapper
{
    long long value;
    struct pointer_t p;
};

int CAS(struct pointer_t *ptr, struct pointer_t old, struct pointer_t new)
{
    union cas_wrapper old_;
    old_.p = old;
    union cas_wrapper new_;
    new_.p = new;
    return __sync_bool_compare_and_swap((long long *)ptr, old_.value, new_.value);
}

int is_equal(struct pointer_t lhs, struct pointer_t rhs)
{
    return (lhs.ptr == rhs.ptr && lhs.count == rhs.count);
}

struct node
{
    int value;
    struct pointer_t next;
};

struct queue
{
    struct pointer_t head;
    struct pointer_t tail;
};

#include <stdlib.h>
void initialize(struct queue *Q)
{
    struct node *node = calloc(1, sizeof(struct node));
    node->next.ptr = NULL;
    Q->head.ptr = node;
    Q->tail.ptr = node;
}

void destroy(struct queue *Q)
{
    struct node *curr = Q->head.ptr, *tmp;
    while (curr->next.ptr)
    {
        tmp = curr;
        curr = curr->next.ptr;
        free(tmp);
    }
}

void enqueue(struct queue *Q, int value);
int dequeue(struct queue *Q, int *value);

void *inc(void *arg);
void *dec(void *);

#include <pthread.h>
struct context
{
    pthread_attr_t attr;
    int shared;
    pthread_t t1, t2;
    struct queue q;
};

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
#include <stdio.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    initialize(&ctx->q);
}
#include <assert.h>
void ctx_destroy(struct context *ctx)
{
    pthread_join(ctx->t1, NULL);
    pthread_join(ctx->t2, NULL);
    pthread_attr_destroy(&ctx->attr);
    assert(is_equal(ctx->q.head, ctx->q.tail));
    destroy(&ctx->q);
}

#include <stdio.h>
#include <unistd.h>
#define ITER 5
void *inc(void *arg)
{
    struct context *ctx = (struct context *)arg;
    for (int i = 0; i < ITER; ++i)
    {
        enqueue(&ctx->q, i);
        printf("enqueue %d\n", i);
        sleep(5);
    }
    pthread_exit((void *)0);
}

void *dec(void *arg)
{
    struct context *ctx = (struct context *)arg;
    int val;
    for (int i = 0; i < ITER; ++i)
    {
        if (dequeue(&ctx->q, &val))
            printf("dequeue %d\n", val);
        else
        {
            printf("✗ dequeue failed\n");
            --i;
        }
        sleep(1);
    }
    pthread_exit((void *)0);
}


#include <stdio.h>
void enqueue(struct queue *Q, int value)
{
    struct node *node = malloc(sizeof(struct node));
    node->value = value;
    node->next.ptr = NULL;
    for (;;)
    {
        struct pointer_t tail = Q->tail;
        struct pointer_t next = tail.ptr->next;
        if (is_equal(tail, Q->tail))
        {
            if (next.ptr == NULL)
            {
                struct pointer_t new_next =
                    {
                        node, next.count + 1};

                if (CAS(&tail.ptr->next, next, new_next))
                {
                    struct pointer_t new_tail =
                        {
                            node,
                            tail.count + 1};
                    CAS(&Q->tail, tail, new_tail);
                    break;
                }
            }
            else
            {
                struct pointer_t new_tail =
                    {
                        next.ptr, tail.count + 1};
                CAS(&Q->tail, tail, new_tail);
            }
        }
        else
        {
        }
    }
}

int dequeue(struct queue *Q, int *value)
{
    for (;;)
    {
        struct pointer_t head = Q->head;
        struct pointer_t tail = Q->tail;
        struct pointer_t next = head.ptr->next;
        if (is_equal(head, Q->head))
        {
            if (head.ptr == tail.ptr)
            {
                if (next.ptr == NULL)
                {
                    return 0;
                }
                else
                {
                    struct pointer_t new_tail = {
                        next.ptr, tail.count + 1};
                    CAS(&Q->tail, tail, new_tail);
                }
            }
            else
            {
                *value = next.ptr->value;
                struct pointer_t new_head = {
                    next.ptr,
                    head.count + 1};
                if (CAS(&Q->head, head, new_head))
                {
                    free(head.ptr);
                    break;
                }
            }
        }
    }
    return 1;
}

