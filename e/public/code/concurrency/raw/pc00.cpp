// 用条件变量实现生产消费者模式
// =========================
//
// 设计:
//
// * 任务队列
// * 一个生产者往队列放入任务, 如果队列满了, 通过 `pthread_cond_wait()` 等待一个条件变量 `free_slot`
// * 多个消费者往队列取出任务, 如果队列为空, 通过 `pthread_cond_wait()` 等待 `available_task`
// * 放入和取出任务的时候需要加锁

// 首先要考虑队列放在哪, 统一放在 Factory 类, Factory 包含消费者线程池和生产者,
//  任务队列, 条件变量, 锁, 就跟上一节的 `struct context` 一样
#include <pthread.h>
#include <queue>
#include <vector>
#define CONSUMER_NUM 4
#define MAX_TASK 8
using namespace std;

struct Task;
struct Producer;
struct Consumer;
struct Factory
{
    // 如果队列满了, 生产者等待条件变量 `free_slot`
    pthread_cond_t free_slot;
    // 如果队列空了, 消费者等待条件变量 `available_task`
    pthread_cond_t available_task;
    queue<Task> tasks;
    int max_tasks;
    // 保障队列的读写
    pthread_mutex_t guard;

    Producer *producer;
    vector<Consumer *> consumers;

    pthread_attr_t attr;
    pthread_t pid;
    pthread_t cid[CONSUMER_NUM];

    Factory();
    ~Factory();
    void set_producer(Producer *p)
    {
        producer = p;
    }
    void add_comsumer(Consumer *c)
    {
        consumers.push_back(c);
    }
    // 运行工厂
    void run();
};

struct Producer
{
    Factory *fac;
    Producer(Factory *f) : fac(f){};
    // 每 0.1 秒往队列放入一个整数
    static void *run(void *arg);
};

struct Consumer
{
    Factory *fac;
    int id;
    Consumer(Factory *f, int i) : id(i), fac(f){};
    static void *run(void *arg);
};

// `main()` 函数
int main()
{
    Factory factory;
    Producer *p = new Producer(&factory);
    factory.set_producer(p);
    for (int i = 0; i < 8; ++i)
    {
        Consumer *con = new Consumer(&factory, i);
        factory.add_comsumer(con);
    }
    factory.run();
}

// 任务是打印数字;
struct Task
{
    Task(int i) : val(i) {}
    int val;
};

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <unistd.h>

// 每 0.1 秒往队列放入一个整数
void *Producer::run(void *arg)
{
    Producer *p = (Producer *)arg;
    Factory *fac = p->fac;
    for (;;)
    {
        int r = std::rand() % 20;
        pthread_mutex_lock(&fac->guard);
        {
            // 防止 spurious wakeup
            while (fac->tasks.size() == fac->max_tasks)
            {
                std::cout << "[Producer]: waiting for free slots" << std::endl;
                pthread_cond_wait(&fac->free_slot, &fac->guard);
                std::cout << "[Producer]: got a free slot" << std::endl;
            }
            std::cout << "[Producer] -> " << r << std::endl;
            fac->tasks.emplace(r);
            // 只要生产, 必有任务, 那就应该通知在等任务的线程
            pthread_cond_signal(&fac->available_task);
        }
        pthread_mutex_unlock(&fac->guard);
        sleep(0.1);
    }
}

void *Consumer::run(void *arg)
{
    Consumer *p = (Consumer *)arg;
    Factory *fac = p->fac;
    for (;;)
    {
        pthread_mutex_lock(&fac->guard);
        {
            while (0 == fac->tasks.size())
            {
                std::cout << "[Comsumer " << p->id << "]"
                          << ": no task, wait" << std::endl;
                pthread_cond_wait(&fac->available_task, &fac->guard);
                std::cout << "[Comsumer " << p->id << "]"
                          << ": got a task" << std::endl;
            }
            // 费时操作;
            sleep(3);
            std::cout << "[Comsumer " << p->id << "] <- " << fac->tasks.front().val << std::endl;
            fac->tasks.pop();
            // 只要弹出, 必有空槽, 那就应该通知在等空槽的线程
            pthread_cond_signal(&fac->free_slot);
        }
        pthread_mutex_unlock(&fac->guard);
        sleep(0.1);
    }
}

// 把所有线程都跑起来
void Factory::run()
{
    pthread_create(&this->pid, &this->attr, Producer::run, (void *)producer);
    for (int i = 0; i < CONSUMER_NUM; ++i)
    {
        pthread_create(&this->cid[i], &this->attr, Consumer::run, (void *)(consumers[i]));
    }
}

// pthread 变量初始化和回收
Factory::Factory() : max_tasks(MAX_TASK)
{
    pthread_cond_init(&free_slot, NULL);
    pthread_cond_init(&available_task, NULL);
    pthread_mutex_init(&guard, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}

Factory::~Factory()
{
    pthread_join(pid, NULL);
    for (int i = 0; i < CONSUMER_NUM; ++i)
    {
        pthread_join(cid[i], NULL);
    }
    pthread_cond_destroy(&free_slot);
    pthread_cond_destroy(&available_task);
    pthread_mutex_destroy(&guard);
    pthread_attr_destroy(&attr);
    delete (producer);
    for (int i = 0; i < CONSUMER_NUM; ++i)
    {
        delete (consumers[i]);
    }
}

// 
// ```
// ➜  g++ -g -O0 -o pc00 pc00.cpp -pthread
// ➜  ./pc00
// [Producer] -> 3
// [Comsumer 0] <- 3
// [Comsumer 1]: no task, wait
// [Comsumer 0]: no task, wait
// [Comsumer 3]: no task, wait
// [Producer] -> 6
// [Comsumer 1]: got a task
// [Comsumer 1] <- 6
// [Comsumer 1]: no task, wait
// [Comsumer 2]: no task, wait
// [Producer] -> 17
// [Comsumer 0]: got a task
// [Comsumer 0] <- 17
// [Comsumer 0]: no task, wait
// [Producer] -> 15
// [Comsumer 3]: got a task
// [Comsumer 3] <- 15
// [Producer] -> 13
// [Comsumer 3] <- 13
// [Comsumer 1]: got a task
// [Comsumer 1]: no task, wait
// [Producer] -> 15
// [Comsumer 3] <- 15
// [Comsumer 2]: got a task
// [Comsumer 2]: no task, wait
// [Producer] -> 6
// [Comsumer 0]: got a task
// [Comsumer 0] <- 6
// [Comsumer 3]: no task, wait
// [Comsumer 0]: no task, wait
// [Producer] -> 12
// [Comsumer 1]: got a task
// [Comsumer 1] <- 12
// [Producer] -> 9
// [Comsumer 2]: got a task
// [Comsumer 2] <- 9
// [Comsumer 1]: no task, wait
// [Producer] -> 1
// [Comsumer 0]: got a task
// [Comsumer 0] <- 1
// [Comsumer 2]: no task, wait
// [Producer] -> 2
// [Comsumer 3]: got a task
// [Comsumer 3] <- 2
// [Comsumer 0]: no task, wait
// [Comsumer 3]: no task, wait
// [Producer] -> 7
// [Comsumer 1]: got a task
// [Comsumer 1] <- 7
// [Producer] -> 10
// [Comsumer 2]: got a task
// [Comsumer 2] <- 10
// [Comsumer 1]: no task, wait
// [Comsumer 2]: no task, wait
// [Producer] -> 19
// [Comsumer 0]: got a task
// ^C
// ```

