// C++ Asynchrony
// ===================
//
// 
// 了解完 pthread 互斥变量, 条件变量等同步原语之后, 就没必要再看它们的 C++ 版本; C++ 做了很多事情, 尤其 `std::atomic` 会根据类型决定最终到底用
//   atomic memory access 还是 memory fence(锁), 可以查看类型:
//
// ```cpp
// std::atomic<T> a;
// cout << c.is_lock_free() << endl;
// ```
//
// 如果无锁, 就是 Lock-free. 其中有很多细节, 但完全可以通过查文档解决. 
//
// 本文关注的是 C++ 的异步编程. 我是从 Rust 开始才关注异步的[^1](/e/public/posts/asynchrony.html),
// 但 Rust 的语法门槛太高, 从 C++ 开始也许更好.
//

// `std::aync`
// ------------------
//
// [https://en.cppreference.com/w/cpp/thread/async](https://en.cppreference.com/w/cpp/thread/async)
//
// 运行一个异步函数, 马上返回结果; Promise/Future 某种程度上就和风筝一样, 通过一条通道(线)和放风筝的人通信, 但放风筝的人只能兑现一次 Promise, 完了之后再兑现一次就报错, 相当于线断了(什么比喻?);
//
// 接口:
// ```cpp
// using T = std::future<std::invoke_result_t<std::decay_t<Function>,
//                                  std::decay_t<Args>...>>;
// T async( Function&& f, Args&&... args );
// T async( std::launch policy, Function&& f, Args&&... args );
// ```
//
// `policy`:
// 
// 1. `std::launch::async`: 执行起来就和 `std::thread` 一样
// 2. `std::launch::deferred`: 不开启新线程, 而是 lazy evaluation, 第一次 wait() 的时候才开始运行
//
// [Promise](https://en.cppreference.com/w/cpp/thread/promise), [Future](https://en.cppreference.com/w/cpp/thread/future)
// --------------------------
//
// `promise` 和 `future` 是管道的两端, `future` 在 caller 手中; 通过 `promise::get_future()` 获取 `future`;
// 这两者都是对锁, 和条件变量, 或者信号量的封装, 用 `strace` 追踪能看到 `FUTEX` 的调用.
// 完全可以把管道视为只有一个生产者一个消费者, 只能使用一次的队列.
//

// 接下来用 C++ 实现一个计时器, 可惜的是, future/promise 是一次性(one-shot)的同步方法, 如果涉及多个定时任务, 
// 就不得不跟 [The Rust Aync Book](https://rust-lang.github.io/async-book/) 一样轮询 `future` 队列, 这种代码写了没什么意义;
// 如果不用轮询的方法, 那还有一种方法, 涉及到条件变量:
//
// 1. 主线程, 检查已经完成的工作队列, 如果总数为 0, 进入 `pthread_cond_wait()`
// 2. 工作线程, 完成睡眠任务之后, 往队列放入元素, 并 `pthread_cond_sign()`
// 3. 主线程醒来, 给用户返回反馈
//
// 然而这就是生产消费者模式, 已经实现过了: [用条件变量实现生产消费者模式](/e/public/code/concurrency/producer-consumer.cpp.html)
//
// 因此这个例子仅仅用于记载 C++ Promise/Future 的用法

#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <iostream>
#include <chrono>

void sleep(int sec, std::promise<void> promise)
{
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    // 不需要给 `future` 返回数据, 所以参数为空, 对应 `<void>`
    promise.set_value();
}

int main(int argc, char** argv)
{
    int secs = std::stoi(argv[1]);
    std::promise<void> sleep_promise;
    std::future<void> sleep_future = sleep_promise.get_future();
    // 移动, 是因为 promise is not copy-assignable [https://en.cppreference.com/w/cpp/thread/promise/operator%3D](https://en.cppreference.com/w/cpp/thread/promise/operator%3D);
    // 拷贝函数被删除了
    std::thread sleep_thread(sleep, secs, std::move(sleep_promise));
    sleep_future.wait();
    std::cout << "Time's up: " << secs << " seconds elapsed" << std::endl;
    sleep_thread.join();
}