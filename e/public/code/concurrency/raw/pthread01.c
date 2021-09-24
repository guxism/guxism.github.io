// 条件变量
// ============
//
// 条件变量是线程间通信的一种方法, 具体表现为等待条件变量不可用的时候,
// 线程进入睡眠状态, 直到另一个线程把它唤醒
//
// 在这个例子中:
//
// * 2 个线程 A, B 对一个变量 V 进行加减操作, 一个 `+1`, 一个 `-1`,
// * 第三个线程 C 检查变量, 检测到 `V == 0` 的时候, 打印"好耶", 否则就进入睡眠状态
// * 当 A, B 发现 `V == 0` 的时候, 需要唤醒 C
//

// 把 [mutex.c](mutex.c.html) 的代码拿过来用;
// 这俩是线程 A, B 执行的函数
void *inc(void *arg);
void *dec(void *);
// 线程 C
void *watch(void *);

// 用 context 提供可重入性
#include <pthread.h>
struct context
{
    // Condition Variable, 提供唤醒线程的功能;
    // 可以理解为: 条件变量背后有个等待线程的集合;
    // 通过 `pthread_cond_signal(&cv)`, 可以唤醒集合里面的线程
    pthread_cond_t cv;
    // 还是需要一个共享变量; 简单示例用 int, 显示用更复杂的数据结构,
    // `pthread_cond_t` 没法应付这么多需求
    int shared;
    // 用互斥变量保护共享变量的读写
    pthread_mutex_t mutex;
    // 标记是谁发送信号
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

// 上下文初始化和回收的实现
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

// 整个过程包括上下文初始化, 运行, 上下文回收;
int main()
{
    struct context ctx;
    ctx_init(&ctx);
    run(&ctx);
    ctx_destroy(&ctx);
    pthread_exit(NULL);
}

// 写完以上的 boilerplant 之后进入正题;
#include <stdio.h>
#include <unistd.h>
void *inc(void *arg)
{
    struct context *ctx = (struct context *)arg;
    pthread_mutex_lock(&ctx->mutex);
    // 做一件会被打断的事情;
    int read = ctx->shared;
    printf("inc: read %d\n", ctx->shared);
    sleep(0.5);
    int write = read + 1;
    ctx->shared = write;
    printf("inc: write %d\n", ctx->shared);
    // 如果发现 shared == 0, 唤醒线程 C
    if (ctx->shared == 0)
    {
        printf("inc signal\n");
        ctx->signaller = 'A';
        // 发出信号之后, 线程马上就能解开 ctx->mutex;
        // 这时信号到达线程 C(操作系统唤醒), C 有希望比 A, B 更快获得 ctx->mutex;
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
    // 如果刚好是 0 则跳过; 用 `while` 而不是 `if`, 是为了防止 spurious wakeup;
    // spurious wakeup 无解, 把它看作操作系统的失误即可.
    while (ctx->shared != 0)
    {
        printf("Poll failed, enter waiting state\n");
        // 等待的时候, 放弃 ctx->mutex; 这个步骤在 Pthread 内部实现, 我们看不到;
        // 线程 C 被唤醒之后, 马上获得锁, 并从 `pthread_cond_wait()` 返回;
        pthread_cond_wait(&ctx->cv, &ctx->mutex);
        printf("%c me excitat!\n", ctx->signaller);
    }
    // 线程 C 执行任务:
    printf("好耶!\n");
    pthread_mutex_unlock(&ctx->mutex);
    pthread_exit((void *)0);
}

// 编译:
//
// ```bash
// gcc -O0 -o mutex condition-variable.c -pthread
// ```
//
// 运行结果:
//
// ```
// ➜  .out/condition-variable 
// dec: read 0
// dec: write -1
// Poll failed, enter waiting state
// inc: read -1
// inc: write 0
// inc signal
// A me excitat!
// 好耶!
// ctx->shared: 0
// ```
//
// 这个例子有点简单了, 本想设计成多次循环, 但可能会出现线程 C 无限阻塞的情况.
// 设想, 如果让每个线程只迭代 10 次, 程序运行之后, A 连续 +10, B 连续 -10, A, B 一共消耗了 10 次迭代,
// C 堵塞从第 1 轮迭代的等待中醒来, 这是可能发生的情况;
// 最后 A, B 退出, C 还在等. 可以用 `pthread_cond_timewait()` 等待一定时间后退出.
// 但作为简单的接口使用示例, 就到此为止, 读者可以自行扩展.
//
// 最后总结条件变量的使用协议:
// <table>
//  <tr>
//      <td>Thread A</td>
//      <td>Thread B</td>
//  </tr>
//  <tr>
//      <td> lock mutex <br/>signal cv <br/> unlock mutex</td>
//      <td> lock mutex <br/>wait(cv, mutex)   <br/> unlock mutex</td>
//  </tr>
// </table>
//


