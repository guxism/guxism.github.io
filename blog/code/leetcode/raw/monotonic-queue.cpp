
auto win_front = nums.begin();
auto win_back = nums.begin();
int max = *win_front;
FULL_FILL_A_WINDOW();
while (win_front < nums.end())
{
    if (*win_back != max)
        ;
}


struct Tracer
{
    Tracer() : empty(true) {}
    int max;
    bool empty;
    multiset<int> set;
    void push(int x)
    {
        if (empty)
        {
            max = x;
        }
        else if (x > max)
        {
            set.insert(max);
            max = x;
        }
        else
        {
            set.insert(x);
        }
        empty = false;
    }
    void pop(int x)
    {
        if (x == max)
        {
            if (!set.empty())
            {
                auto i = set.end();
                --i;
                max = *i;
                set.erase(i);
            }
            else
            {
                empty = true;
            }
        }
        else if (!set.empty())
        {
            auto i = set.find(x);
            set.erase(i);
        }
    }
};

public:
vector<int> maxSlidingWindow(vector<int> &nums, int k)
{
    if (nums.empty() || k <= 0)
        return {};
    auto front = nums.begin();
    auto back = nums.begin();
    Tracer tracer;
    vector<int> res;
    tracer.push(*front++);
    while (front - back < k)
    {
        tracer.push(*front);
        ++front;
    }
    res.push_back(tracer.max);
    while (front < nums.end())
    {
        tracer.pop(*back);
        ++back;
        tracer.push(*front);
        ++front;
        res.push_back(tracer.max);
    }
    return res;
}


struct Tracer
{
    Tracer(int k) : buf(new int[k]), head(0), cap(k), bufsize(0) {}
    ~Tracer() { delete[] buf; }
    int *buf;
    int head;
    int cap;
    int bufsize;
    inline int top() { return buf[head]; }
    void push(int x)
    {
        if (!bufsize || x > buf[head])
        {
            buf[head] = x;
            bufsize = 1;
        }
        else
        {
            int tail = head + bufsize - 1 + cap;
            while (tail % cap != head && buf[tail % cap] < x)
            {
                --bufsize;
                --tail;
            }
            buf[(tail + 1) % cap] = x;
            ++bufsize;
        }
    }
    void pop()
    {
        if (bufsize)
        {
            head = (head + 1) % cap;
        }
        --bufsize;
    }
};
vector<int> maxSlidingWindow(vector<int> &nums, int k)
{
    if (nums.empty() || k <= 0 || nums.size() < (size_t)k)
        return {};

    auto front = nums.begin();
    auto back = nums.begin();
    Tracer t(k);
    vector<int> res;
    t.push(*front++);
    while (front - back < k)
    {
        t.push(*front);
        ++front;
    }
    res.push_back(t.top());
    while (front < nums.end())
    {
        if (t.top() == *back)
        {
            t.pop();
        }
        ++back;

        t.push(*front);
        ++front;
        res.push_back(t.top());
    }
    return res;
}

