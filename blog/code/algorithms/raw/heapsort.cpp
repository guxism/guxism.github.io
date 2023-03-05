
#include "libcxx.h"
using namespace std;

inline int PARENT(int i)
{
    return i / 2;
}

inline int LEFT(int i)
{
    return 2 * i;
}

inline int RIGHT(int i)
{
    return 2 * i + 1;
}

struct Heap
{
    Heap(const vector<int> &src): arr(src), heap_size(src.size()), length(heap_size)
    {
    }
    int &operator[](int i)
    {
        return arr[i-1];
    }
    vector<int> arr;
    int heap_size;
    int length;
};

void MAX_HEAPIFY(Heap &A, int i)
{
    int left = LEFT(i);
    int right = RIGHT(i);
    int largest = i;
    if (left <= A.heap_size && A[left] > A[i])
    {
        largest = left;
    }
    if (right <= A.heap_size && A[right] > A[largest])
    {
        largest = right;
    }
    if (largest != i)
    {
        swap(A[i], A[largest]);
        MAX_HEAPIFY(A, largest);
    }
}

void BUILD_MAX_HEAP(Heap &A)
{
    A.heap_size = A.length;
    for (int i = (A.length / 2); i >= 1; --i)
    {
        MAX_HEAPIFY(A, i);
    }
}

vector<int> heap_sort(const vector<int> &src)
{
    Heap A(src);
    BUILD_MAX_HEAP(A);
    for (int i = A.length; i >= 2; --i)
    {
        swap(A[1], A[i]);
        A.heap_size -= 1;
        MAX_HEAPIFY(A, 1);
    }
    return std::move(A.arr);
}

int main()
{
    sort_test(heap_sort);
}
