
#include "libcxx.h"

using namespace std;

void _merge(vector<int> &src, int left, int mid, int right);
vector<int> merge_sort(const vector<int> &src)
{
    vector<int> res{src};
    for (size_t size = 1; size < src.size(); size *= 2)
    {
        for (
            size_t left = 0;
            left < src.size();
            left = left + size * 2)
        {
            auto mid = std::min(left + size - 1, src.size() - 1);
            auto right = std::min(left + 2 * size - 1, src.size() - 1);
            _merge(res, left, mid, right);
        }
    }
    return res;
}

void _merge(vector<int> &src, int left, int mid, int right)
{
    int n1 = mid - left + 1;
    int n2 = right - (mid + 1) + 1;
    int part_1[n1];
    int part_2[n2];
    for (int i = 0; i < n1; ++i)
    {
        part_1[i] = src[i + left];
    }
    for (int i = 0; i < n2; ++i)
    {
        part_2[i] = src[i + mid + 1];
    }
    int r1 = 0, r2 = 0;
    int w = left;
    while (r1 < n1 && r2 < n2)
    {
        if (part_1[r1] <= part_2[r2])
        {
            src[w] = part_1[r1];
            ++r1;
        }
        else
        {
            src[w] = part_2[r2];
            ++r2;
        }
        ++w;
    }
    while (r1 < n1)
    {
        src[w++] = part_1[r1++];
    }

    while (r2 < n2)
    {
        src[w++] = part_2[r2++];
    }
}

int main()
{
    sort_test(merge_sort);
}

