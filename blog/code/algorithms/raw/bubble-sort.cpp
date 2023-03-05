
#include "libcxx.h"

using namespace std;

vector<int> bubble(const vector<int> &src)
{
    vector<int> res{src};
    for (auto last_index = src.size() - 1; last_index > 0; --last_index)
    {
        for (auto curr = 0; curr < last_index; ++curr)
        {
            if (res[curr] > res[curr + 1])
                swap(res[curr], res[curr + 1]);
        }
    }
    return res;
}

int main()
{
    sort_test(bubble);
}

