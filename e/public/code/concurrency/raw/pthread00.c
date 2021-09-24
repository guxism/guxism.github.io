// POSIX Threads Programming
// ===========================
//
// 本文提供了互斥变量的一个例子. 如果想了解 Pthread 的细节, 我推荐 [Lawrence Livermore National Laboratory 的教程](https://hpc-tutorials.llnl.gov/posix/).
//
//

// 互斥变量
// -------------------------
//
// 如果没有互斥变量, 就会造成竞争条件: 设想, 对于变量 V, 初始值为 0
// 
// * 线程 A 读取变量, 将值存放在 V', 准备 +1 之时, 被线程 B 抢占,
// * 线程 B 读取变量 V 并 +1, 此时 V == 1
// * 线程 A 重夺控制权, 将它读到的变量 V' + 1, 然后写入 V, 此时 V == 1
// 
// V 的期望值是 2, 但没有互斥变量的保护, V 得到错误的值.
//
// 下面的例子用两个线程实现一个变量的增加和递减
// 一个线程加 1, 一个线程减 1, 最后变量的值不变
//

// 已知 pthread_create 的接口
// ```c
// int pthread_create(pthread_t *thread,
//          const pthread_attr_t *attr,
//          void *(*start_routine)(void *), void *arg);
// ```

// 那么现在就可以确定回调函数的接口.
void *inc(void *arg);
void *dec(void *);

// 将 inc 的实际参数设置为 struct context* 类型, 而不是 `int*`.
// 这么做的好处在于可以把互斥变量和操作对象一起传进去
struct context;
void run(struct context *ctx);

#include <pthread.h>
struct context
{
    pthread_attr_t attr;
    int shared;
    // 这个例子只需要两条线程, 所以直接写死, 不然还需要写个 vector
    pthread_t t1, t2;
    // 关键点: 互斥变量
    pthread_mutex_t mutex;
};

// 确定执行过程
void run(struct context *ctx)
{
    pthread_create(&ctx->t1, &ctx->attr, inc, (void *)ctx);
    pthread_create(&ctx->t2, &ctx->attr, dec, (void *)ctx);
}

// 上下文的初始化和回收
void ctx_init(struct context *ctx);
void ctx_destroy(struct context *ctx);

// 整个过程包括上下文初始化, 运行, 上下文回收
int main()
{
    struct context ctx;
    ctx_init(&ctx);
    run(&ctx);
    ctx_destroy(&ctx);
    pthread_exit(NULL);
}

// 实现 `inc()` 和 `dec()`
#include <stdio.h>
#include <unistd.h>
void *inc(void *arg)
{
    struct context *ctx = (struct context *)arg;
    pthread_mutex_lock(&ctx->mutex);
    // 做一件会被打断的事情;
    // 如果不用 `sleep(2)`, 则很难复现 race condition
    int read = ctx->shared;
    printf("inc: read %d\n", ctx->shared);
    sleep(2);
    int write = read + 1;
    ctx->shared = write;
    printf("inc: write %d\n", ctx->shared);
    pthread_mutex_unlock(&ctx->mutex);
    pthread_exit((void *)0);
}

void *dec(void *arg)
{
    struct context *ctx = (struct context *)arg;
    pthread_mutex_lock(&ctx->mutex);
    int read = ctx->shared;
    printf("dec: read %d\n", ctx->shared);
    sleep(2);
    int write = read - 1;
    ctx->shared = write;
    printf("dec: write %d\n", ctx->shared);
    pthread_mutex_unlock(&ctx->mutex);
    pthread_exit((void *)0);
}

// 上下文初始化和回收的函数实现
#include <strings.h>
void ctx_init(struct context *ctx)
{
    bzero(ctx, sizeof(struct context));
    pthread_attr_init(&ctx->attr);
    pthread_attr_setdetachstate(&ctx->attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&ctx->mutex, NULL);
}

void ctx_destroy(struct context *ctx)
{
    pthread_join(ctx->t1, NULL);
    pthread_join(ctx->t2, NULL);
    pthread_attr_destroy(&ctx->attr);
    pthread_mutex_destroy(&ctx->mutex);
    printf("ctx->shared: %d\n", ctx->shared);
}

// 编译:
//
// ```bash
// gcc -O0 -o mutex mutex.c -pthread
// ```
//
// 为什么用 `-pthread` 而不是 `-lpthread`?
//
// 1. 因为现代 Linux 系统没有 libpthread.so,
// Pthread 由 libc.so 提供, 后者是编译器默认的链接选项. 所以不需要 `-lpthread`
// 2. `-pthread` 是一个编译选项, 告诉 gcc, 代码使用了 Pthread API,
// POSIX 之所以 Potable, 是因为它封装了各平台的多线程接口, `-pthread` 告诉编译器检查当前平台类型, 定义相关的宏变量
// 3. `-pthread` 同时也是一个链接选项, 道理同上, 只链接符合 Pthread 接口规范的库

// 结果:
//
// ```
// ➜  out/mutex
// inc: read 0
// inc: write 1
// dec: read 1
// dec: write 0
// ctx->shared: 0
// ➜  out/mutex
// dec: read 0
// dec: write -1
// inc: read -1
// inc: write 0
// ctx->shared: 0
// ```
//
// 如果不加锁(注释 `pthread_mutex_lock` 和 `pthread_mutex_unlock`), 结果是:
//
// ```
// ➜  out/mutex
// inc: read 0
// dec: read 0
// inc: write 1
// dec: write -1
// ctx->shared: -1
// ```

