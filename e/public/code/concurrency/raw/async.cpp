

#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <iostream>
#include <chrono>

void sleep(int sec, std::promise<void> promise)
{
    std::this_thread::sleep_for(std::chrono::seconds(sec));
    promise.set_value();
}

int main(int argc, char** argv)
{
    int secs = std::stoi(argv[1]);
    std::promise<void> sleep_promise;
    std::future<void> sleep_future = sleep_promise.get_future();
    std::thread sleep_thread(sleep, secs, std::move(sleep_promise));
    sleep_future.wait();
    std::cout << "Time's up: " << secs << " seconds elapsed" << std::endl;
    sleep_thread.join();
}

