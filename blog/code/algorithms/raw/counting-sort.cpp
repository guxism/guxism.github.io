#include "libcxx.h"
using namespace std;

vector<int> counting_sort(const vector<int> &src)
{
    vector<int> res(src.size());
    int k = 10;
    int C[k] = {0};
    for (auto& n: src)
    {
        C[n] += 1;
    }
    for (int i = 1; i < k; ++i)
    {
        C[i] += C[i-1];    
    }
    for (int j = 0; j < src.size(); ++j)
    {
        int item = src[j];
        int pos = C[item];
        res[pos-1] = item;
        C[item] -= 1;
    }
    return res;
}

int main()
{
    sort_test(counting_sort);
}




