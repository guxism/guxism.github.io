
#include "graph.h"
#include <stack>
using namespace std;
using namespace graph;

void dfs(Graph &g, std::string start);

int main(int argc, char **argv)
{
    string graph = R"(
        digraph A {
            ratio=fill
            splines=spline
            overlap=scale
            rankdir=LR 
            node [margin=0, shape=circle, penwidth=2]
            u -> x
            u -> v
            x -> v
            v -> y
            y -> x
            w -> y
            w -> z
            z -> z
        }
    )";
    auto g = graph::interpret(graph);

    g->display();
    dfs(*g, "u");
    ofstream of("graph_dfs.dot");
    export_graph2(*g, of);
}

void dfs_visit(Graph &g, std::string start, unsigned int& time);
void dfs(Graph &g, std::string start)
{
    unsigned int time = 0;
    g.cleanup_color();
    dfs_visit(g, start, time);
    for (auto& v: g.adj_list)
    {
        if (v.second.first.color == color::WHITE)
            dfs_visit(g, v.second.first.name, time);
    }
}

void dfs_visit(Graph &g, std::string start, unsigned int& time)
{

    std::stack<std::string> s;
    Vertex &start_vertex = g.get_vertex(start).first;

    if (start_vertex.color == color::BLACK)
    {
        return;
    }


    ++time;
    start_vertex.turn_gray(time);
    s.push(start);

loop:
    while (!s.empty())
    {
        std::string name = s.top();
        VertexAndNeighbors &vn = g.get_vertex(name);
        auto &curr = vn.first;
        auto neighbors = vn.second;

        if (neighbors.size() == 0)
        {
            ++time;
            curr.turn_black(time);
            s.pop();
            continue;
        }

        for (auto &neigh : neighbors)
        {
            if (neigh.dest->color == color::WHITE)
            {
                neigh.dest->dist = curr.dist + neigh.weight;
                neigh.dest->prev = &curr;
                ++time;
                neigh.dest->turn_gray(time);
                s.push(neigh.dest->name);
                goto loop;
            }
        }
        ++time;
        curr.turn_black(time);
        s.pop();
    }
}

