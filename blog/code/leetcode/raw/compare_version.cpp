
#include <cassert>
#include <string>
#include <vector>

using namespace std;

vector<char> vectorize(const string &str)
{
    vector<char> res;
    res.push_back('.');
    for (auto i = str.begin(); i != str.end(); ++i)
    {
        if (*i == '0')
        {
            if (res.back() != '.')
            {
                res.push_back(*i);
            }
        }
        else
        {
            res.push_back(*i);
        }
    }
    return res;
}

int compare_failed(const string &v1, const string &v2)
{
    auto vec1 = vectorize(v1);
    auto vec2 = vectorize(v2);

    size_t i = 0;
    while (i < vec1.size() && i < vec2.size())
    {
        if (vec1[i] > vec2[i])
            return 1;
        else if (vec1[i] < vec2[i])
            return -1;
        ++i;
    }
    if (vec1.size() == vec2.size())
    {
        return 0;
    }
    else if (vec1.size() > vec2.size())
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

int compare(const string &v1, const string &v2)
{
    auto vec1 = vectorize(v1);
    auto vec2 = vectorize(v2);
    vec1.push_back('.');
    vec2.push_back('.');

    auto p1 = vec1.begin();
    auto p2 = vec2.begin();

    p1++;
    p2++;

    while (p1 != vec1.end())
    {
        auto e1 = p1;
        auto e2 = p2;

        while (*e1 != '.')
            ++e1;
        while (*e2 != '.')
            ++e2;

        auto len1 = e1 - p1;
        auto len2 = e2 - p2;

        if (len1 != len2)
        {
            if (len1 > len2)
                return 1;
            else if (len1 < len2)
                return -1;
        }
        while (p1 <= e1)
        {
            if (*p1 > *p2)
            {
                return 1;
            }
            else if (*p1 < *p2)
            {
                return -1;
            }
            ++p1;
            ++p2;
        }

    }

    return 0;
}


int main()
{
    assert(compare("1.0.2", "1.0.1") == 1);
    assert(compare("1.00000.2", "1.0.1") == 1);
    assert(compare("1.000001.1", "1.1.1") == 0);
    assert(compare("1.10.2", "1.0.1") == 1);
    assert(compare("0000001.0.2", "1.0.1") == 1);
    assert(compare("1.0.2", "1.0.10") == -1);
    assert(compare("4842213554626336770476399219479.60375244791816719939475323470794.2", 
                   "4842213554626336770476399219479.60375244791816719939475323470794.10") == -1);
    assert(compare("4842213554626336770476399219479.70375244791816719939475323470794.2",
                   "4842213554626336770476399219479.60375244791816719939475323470794.10") == 1);
    assert(compare("4842213554626336770476399219479.60375244791816719939475323470794.010",
                   "4842213554626336770476399219479.60375244791816719939475323470794.10") == 0);
}
