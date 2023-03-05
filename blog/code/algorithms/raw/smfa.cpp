


#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;
using state_t = size_t;
using input_t = char;
using tmap_t = map<state_t, map<input_t, state_t>>;

inline size_t index(state_t n) { return n - 1; }

tmap_t compute_transition_map(string pattern, set<input_t> charset)
{
    tmap_t tmap;
    state_t m = pattern.size();
    for (state_t q = 0; q <= m; ++q)
    {
        for (auto a : charset)
        {
            size_t k = min(m + 1, q + 2);
            do
            {
                k -= 1;
            } while (
                !(
                    [=]()
                    {
                        string pqa = pattern.substr(0, q) + a;
                        string pk = pattern.substr(0, k);
                        for (auto i = pk.rbegin(), j = pqa.rbegin();
                             i != pk.rend() && j != pqa.rend(); ++i, ++j)
                        {
                            if (*i != *j)
                            {
                                return false;
                            }
                        }
                        return true;
                    }()) &&
                k > 0);
            tmap[q][a] = k;
        }
    }
    return tmap;
}

void finite_automaton_matcher(string T, tmap_t &tmap, state_t accepting_state)
{
    for (size_t i = 1, q = 0; i <= T.size(); ++i)
    {
        input_t a = T[index(i)];
        q = tmap[q][a];
        if (q == accepting_state)
        {
            cout << "Pattern occurs with shift " << (i - accepting_state)
                 << endl;
        }
    }
}

void kmp_hook();
int main()
{
    string P = "ababaca";
    string T = "abababacaba";
    auto charset = [](string &text)
    {
        set<input_t> res;
        for (auto i : text)
        {
            res.insert(i);
        }
        return res;
    }(T);

    auto tmap = compute_transition_map(P, charset);

    finite_automaton_matcher(T, tmap, P.size());
    kmp_hook();
}


vector<state_t> computer_prefix_function(string pattern)
{
    size_t m = pattern.size();
    vector<state_t> pi(m);
    pi[index(1)] = 0;
    state_t k = 0;
    size_t q;

    for (q = 2; q < m; ++q)
    {
        while (k > 0 && pattern[index(k+1)] != pattern[index(q)])
            k = pi[index(k)];
        if (pattern[index(k+1)] == pattern[index(q)])
            k = k + 1;
        pi[index(q)] = k;
    }
    return pi;
}

void kmp_match(string T, string P)
{
    size_t n = T.size();
    state_t m = P.size();

    auto pi = computer_prefix_function(P);

    state_t q = 0;
    for (size_t i = 1; i <= n; ++i)
    {
        while (q > 0 && P[index(q+1)] != T[index(i)])
        {
            q = pi[index(q)];
        }
        if (P[index(q+1)] == T[index(i)])
        {
            q = q + 1;
        }
        if (q == m)
        {
            cout << "Pattern occurs with shift " << (i - m) << endl;
            q = pi[index(q)];
        }
    }
}

void kmp_hook()
{
    string P = "ababaca";
    string T = "abababacaba";
    kmp_match(T, P);
}

