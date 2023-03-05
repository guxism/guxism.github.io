
#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>

using namespace std;

template <typename key_t, typename value_t> struct LFUCache
{
    struct CacheNode;
    struct FreqNode;
    struct CacheNode
    {
        CacheNode() {}
        int value;
        list<FreqNode>::iterator fnode_iter;
    };
    using MapIter = unordered_map<key_t, CacheNode>::iterator;
    struct FreqNode
    {
        FreqNode() : freq(0) {}
        FreqNode(unsigned fq, MapIter iter) : freq(fq)
        {
            map_iters.push_back(iter);
        }
        unsigned freq;
        list<MapIter> map_iters;
    };
    unordered_map<key_t, CacheNode> hashmap;
    list<FreqNode> freq_list;
    size_t cache_limit;

    LFUCache(size_t size) : cache_limit(size) { hashmap.reserve(size * 2); }

    void update_freq(MapIter iter)
    {
        CacheNode &cnode = iter->second;
        typename list<FreqNode>::iterator fnode_iter = cnode.fnode_iter;
        list<MapIter>& map_iters = fnode_iter->map_iters;
        for (auto i = map_iters.begin();
             i != map_iters.end(); std::advance(i, 1))
        {
            if (*i == iter)
            {
                map_iters.erase(i);
                break;
            }
        }
        typename list<FreqNode>::iterator next_fnode_iter = fnode_iter;
        auto curr_freq = cnode.fnode_iter->freq;
        if (map_iters.empty())
        {
            next_fnode_iter = freq_list.erase(fnode_iter);
        }
        else 
        {
            next_fnode_iter = std::next(cnode.fnode_iter, 1);
        }
        
        if (next_fnode_iter != freq_list.end())
        {
            if (next_fnode_iter->freq == curr_freq + 1)
            {
                cnode.fnode_iter = next_fnode_iter;
                next_fnode_iter->map_iters.emplace_back(iter);
                return;
            }
        }
        FreqNode fnode(curr_freq + 1, iter);
        next_fnode_iter = freq_list.insert(next_fnode_iter, move(fnode));
        cnode.fnode_iter = next_fnode_iter;
    }

    void delete_lfu()
    {
        auto front = freq_list.front();
        vector<key_t> to_delete;
        for (auto i : front.map_iters)
        {
            to_delete.push_back(i->first);
        }
        for (auto d : to_delete)
        {
            hashmap.erase(d);
        }
        freq_list.pop_front();
    }

    void insert_new_freq(MapIter iter)
    {
        if (!freq_list.empty() && freq_list.front().freq == 1)
        {
            freq_list.front().map_iters.push_back(iter);
            iter->second.fnode_iter = freq_list.begin();
        }
        else
        {
            FreqNode fnode(1, iter);
            auto fn_iter = freq_list.insert(freq_list.begin(), move(fnode));
            iter->second.fnode_iter = fn_iter;
        }
    }

    bool get(key_t&& key, value_t *v)
    {
        cout << "get key " << key << endl; 
        bool contains_key = hashmap.contains(key);
        bool rc = false;
        if (contains_key)
        {
            MapIter iter = hashmap.find(key);
            *v = iter->second.value;
            update_freq(iter);
            rc = true;
        }
        show();
        return rc;
    }
    void set(key_t&& key, value_t &&v)
    {
        cout << "set key " << key << " with value " << v << endl; 
        MapIter map_iter = hashmap.find(key);
        if (map_iter != hashmap.end())
        {
            update_freq(map_iter);
            show();
            return;
        }
        else if (hashmap.size() == cache_limit)
        {
            delete_lfu();
        }
        CacheNode cnode;
        cnode.value = v;
        MapIter iter = hashmap.insert({key, move(cnode)}).first;
        insert_new_freq(iter);
        show();
    }
    void show()
    {
        for (auto i : freq_list)
        {
            cout << i.freq << ": ";
            for (auto j : i.map_iters)
            {
                cout << "(" << j->first << "->" << j->second.value << ") ";
            }
            cout << endl;
        }
    }
};

int main()
{
    LFUCache<int, int> cache(4);
    cache.set(100, 1);
    cache.set(200, 2);
    cache.set(300, 3);
    cache.set(400, 4);
    int v;
    cache.get(400, &v);
    cache.get(400, &v);
    cache.get(400, &v);
    cache.get(400, &v);
    cache.set(500, 5);
    cache.set(600, 6);
    cache.set(700, 7);
    cache.get(700, &v);
    cache.get(700, &v);
    cache.get(700, &v);
    cache.get(700, &v);
    cache.get(700, &v);
    cache.get(700, &v);
    cache.get(700, &v);
    cache.set(800, 8);

}
