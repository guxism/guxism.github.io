

#include "concurrentqueue.h"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <variant>

struct Message;
using file_tags = std::pair<std::filesystem::path, std::set<std::string>>;
using cq = moodycamel::ConcurrentQueue<Message>;
using statmap = std::map<std::string, std::set<std::filesystem::path>>;
using stat_item = std::pair<std::string, std::set<std::filesystem::path>>;
using TaskValue =
    std::variant<int, std::string, file_tags, stat_item, std::filesystem::path>;
#define LOG(...) printf(__VA_ARGS__)
#define WORKER_THREADS 4

using MesgType = enum MesgType {
    NEW_FILE,
    ALL_FILES_SENT,
    STAT,
    NEW_ROADMAP,
    ROADMAP_CREATED,
    RETIRE
};

struct Message
{
    MesgType type;
    TaskValue value;
};

struct Manager
{
    statmap *stat;
    cq *to_worker;
    cq *to_manager;
    std::string_view root;
    int num_workers;
    Manager(statmap *s, cq *_2w, cq *_2m, int nw, std::string_view r)
        : stat(s), to_worker(_2w), to_manager(_2m), root(r), num_workers(nw)
    {
    }
    Manager() = delete;
};

struct Worker
{
    int id;
    cq *to_worker;
    cq *to_manager;
    std::string_view root;
    Worker(int id, cq *_2w, cq *_2m, std::string_view r)
        : id(id), to_worker(_2w), to_manager(_2m), root(r)
    {
    }
    Worker() = delete;
};

file_tags parse(std::filesystem::path &file);
void create_roadmap(std::string_view &root, stat_item &tag);
void do_work(Worker w)
{
    for (;;)
    {
        Message task;
        if (w.to_worker->try_dequeue(task))
        {
            switch (task.type)
            {
            case MesgType::NEW_FILE:
            {
                auto file = std::get<std::filesystem::path>(task.value);
                LOG("[thread %d]: Process File <%s>\n", w.id, file.c_str());
                file_tags stat = parse(file);
                task.type = MesgType::STAT;
                task.value = stat;
                w.to_manager->enqueue(task);
                break;
            }
            case MesgType::NEW_ROADMAP:
            {
                auto roadmap = std::get<stat_item>(task.value);
                LOG("[thread %d]: Create Roadmap <%s>\n", w.id,
                    roadmap.first.c_str());
                create_roadmap(w.root, roadmap);
                task.type = MesgType::ROADMAP_CREATED;
                task.value = roadmap.first;
                w.to_manager->enqueue(task);
                break;
            }
            case MesgType::RETIRE:
            {
                LOG("[thread %d]: Exit\n", w.id);
                return;
            }
            default:;
            }
        }
    }
}

void merge(statmap *map, file_tags &item);
void manage(Manager manager)
{
    int stat_cnt = 0;
    int total_stat = INT_MAX;
    Message task;
    while (stat_cnt < total_stat)
    {
        if (manager.to_manager->try_dequeue(task))
        {
            switch (task.type)
            {
            case MesgType::STAT:
            {
                ++stat_cnt;
                merge(manager.stat, std::get<file_tags>(task.value));
                break;
            }
            case MesgType::ALL_FILES_SENT:
            {
                int num = std::get<int>(task.value);
                LOG("[Manager]: Notified by Producer, there will be %d Files\n",
                    num);
                total_stat = num;
                break;
            }
            default:;
            }
        }
    }
    int total_file_work = manager.stat->size();
    std::thread mt(
        [manager]
        {
            Message task;
            for (auto i = manager.stat->begin(); i != manager.stat->end(); ++i)
            {
                task.value = *i;
                task.type = MesgType::NEW_ROADMAP;
                LOG("[Phase 2 Producer]: %s\n", i->first.c_str());
                manager.to_worker->enqueue(task);
            }
        });
    mt.detach();
    int cnt_file_work = 0;
    while (cnt_file_work < total_file_work)
    {
        Message task;
        if (manager.to_manager->try_dequeue(task))
        {
            switch (task.type)
            {
            case MesgType::ROADMAP_CREATED:
            {
                ++cnt_file_work;
                LOG("[Manager]: Roadmap <%s> created\n",
                    std::get<std::string>(task.value).c_str());
                break;
            }
            default:;
            }
        }
    }

    for (int i = 0; i < manager.num_workers; ++i)
    {
        task.type = MesgType::RETIRE;
        manager.to_worker->enqueue(task);
    }
    return;
}

std::vector<std::filesystem::path> glob(std::string_view &path);
void produce(Manager manager)
{
    Message task{MesgType::NEW_FILE, 0};
    auto files = glob(manager.root);
    int num = files.size();
    for (auto f : files)
    {
        task.value = f;
        LOG("[Producer]: New Message: %s\n", f.c_str());
        manager.to_worker->enqueue(task);
    }
    task.type = MesgType::ALL_FILES_SENT;
    task.value = num;
    LOG("[Producer]: All %d files are sent\n", num);
    manager.to_manager->enqueue(task);
}

void help() { std::cout << "zet [dir]" << std::endl; }

int main(int argc, char **argv)
{
    std::string root;
    if (argc > 2)
    {
        help();
        return -1;
    }
    else if (argc == 2)
    {
        root = argv[1];
        if (!std::filesystem::exists(root))
        {
            std::cerr << "zet: " << root << " : No such directory" << std::endl;
        }
    }
    else
    {
        root = "./";
    }

    cq q1;
    cq q2;
    statmap stat;
    Manager manager(&stat, &q1, &q2, WORKER_THREADS, root);
    Manager producer(&stat, &q1, &q2, WORKER_THREADS, root);
    std::vector<std::thread> consumers;
    for (int i = 0; i < 4; ++i)
    {
        Worker worker(i + 1, &q1, &q2, root);
        consumers.push_back(std::thread{do_work, worker});
    }
    std::thread mt(manage, manager);
    std::thread pt(produce, manager);
    pt.detach();
    mt.join();
    for (auto &t : consumers)
    {
        t.join();
    }
#if DEBUG
    for (auto &i : stat)
    {
        std::cout << i.first << std::endl;
        for (auto &f : i.second)
        {
            std::cout << f << " " << std::endl;
        }
        std::cout << std::endl;
    }
#endif
    LOG("%lu RoadMaps Generated\n", stat.size());
}

std::vector<std::filesystem::path> glob(std::string_view &path)
{
    std::vector<std::filesystem::path> files;
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (std::filesystem::symlink_status(entry).type() ==
            std::filesystem::file_type::regular)
        {
            auto file = entry.path();
            if (file.string().ends_with(".md"))
            {
                if (file.string().starts_with("__#"))
                {
                    std::filesystem::remove(file);
                }
                else
                {
                    LOG("[Producer]: %s\n", file.c_str());
                    files.push_back(file);
                }
            }
        }
    }
    return files;
}

file_tags parse(std::filesystem::path &file)
{
    std::ifstream infile(file);
    std::string line;
    std::set<std::string> tags;
    std::regex tag_regex(R"(^(#[a-zA-Z0-9_][a-zA-Z0-9_\-]* *)+$)");
    while (std::getline(infile, line))
    {
        if (std::regex_match(line, tag_regex))
        {
            std::string tag;
            char *pch;
            pch = strtok(const_cast<char *>(line.c_str()), " ");
            while (pch != NULL)
            {
                tag = pch;
                if (tag != "roadmap")
                    tags.insert(tag);
                pch = strtok(NULL, " ");
            }
        }
    }
    return file_tags{file, tags};
}

void create_roadmap(std::string_view &root, stat_item &roadmap)
{
    std::string tag = roadmap.first;
    std::filesystem::path filename(root);
    filename /= std::string("__") + tag + ".md";
    for (auto &c : tag)
        c = std::toupper(c);
    auto files = roadmap.second;
    std::ofstream out(filename, std::ios::out);
    out << "# " << tag << std::endl << std::endl;

    for (auto &file : files)
    {
        out << "- [[" << file.filename().c_str() << "]]" << std::endl;
    }
}

void merge(statmap *map, file_tags &item)
{
    std::string file = item.first;
    auto tags = item.second;
    for (auto &tag : tags)
    {
        map->operator[](tag).insert(file);
    }
}

