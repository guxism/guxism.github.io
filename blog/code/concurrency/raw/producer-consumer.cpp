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
    pthread_cond_t free_slot;
    pthread_cond_t available_task;
    queue<Task> tasks;
    int max_tasks;
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
    void run();
};

struct Producer
{
    Factory *fac;
    Producer(Factory *f) : fac(f){};
    static void *run(void *arg);
};

struct Consumer
{
    Factory *fac;
    int id;
    Consumer(Factory *f, int i) : id(i), fac(f){};
    static void *run(void *arg);
};

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

struct Task
{
    Task(int i) : val(i) {}
    int val;
};

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <unistd.h>

void *Producer::run(void *arg)
{
    Producer *p = (Producer *)arg;
    Factory *fac = p->fac;
    for (;;)
    {
        int r = std::rand() % 20;
        pthread_mutex_lock(&fac->guard);
        {
            while (fac->tasks.size() == fac->max_tasks)
            {
                std::cout << "[Producer]: waiting for free slots" << std::endl;
                pthread_cond_wait(&fac->free_slot, &fac->guard);
                std::cout << "[Producer]: got a free slot" << std::endl;
            }
            std::cout << "[Producer] -> " << r << std::endl;
            fac->tasks.emplace(r);
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
            sleep(3);
            std::cout << "[Comsumer " << p->id << "] <- " << fac->tasks.front().val << std::endl;
            fac->tasks.pop();
            pthread_cond_signal(&fac->free_slot);
        }
        pthread_mutex_unlock(&fac->guard);
        sleep(0.1);
    }
}

void Factory::run()
{
    pthread_create(&this->pid, &this->attr, Producer::run, (void *)producer);
    for (int i = 0; i < CONSUMER_NUM; ++i)
    {
        pthread_create(&this->cid[i], &this->attr, Consumer::run, (void *)(consumers[i]));
    }
}

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



