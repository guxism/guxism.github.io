
#include "utils.h"
using namespace std;

vector<int> insertion_sort(const vector<int> &src)
{
    vector<int> res{src};
    size_t oi = 0;
    while (oi < res.size()-1)
    {
        int pick = oi + 1;
        int i;
        for (i = 0; i < pick; ++i)
        {
            if (res[i] > res[pick])
            {
                break;
            }
        }
        if (i != pick)
        {
            int temp = res[pick];
            for (int j = oi; j >= i; --j)
            {
                swap(res[j + 1], res[j]);
            }
            res[i] = temp;
        }
        ++oi;
    }
    return res;
}

int main() { sort_test(insertion_sort); }