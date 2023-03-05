
#include "graph.h"
#include <queue>
using namespace std;
using namespace graph;

void bfs(Graph &g, std::string start);
int main(int argc, char **argv)
{
    string graph = R"(
        graph A {
            ratio=fill
            splines=true
            overlap=scale
            rankdir=LR 
            node [margin=0, shape=circle, penwidth=2]
            v--r
            r--s
            s--w
            w--t
            w--x
            t--x
            t--u
            x--u
            x--y
            u--y
        }
    )";
    auto g = graph::interpret(graph);

    g->display();
    bfs(*g, "s");
    ofstream of("graph_bfs.dot");
    export_graph(*g, "s", of);
}

void bfs(Graph &g, std::string start)
{
    g.cleanup_color();

    std::queue<std::string> q;
    Vertex &start_vertex = g.get_vertex(start).first;

    unsigned time = 0;

    ++time;
    start_vertex.turn_gray(time);
    q.push(start);
    while (!q.empty())
    {
        std::string name = q.front();
        VertexAndNeighbors &vn = g.get_vertex(name);
        auto &curr = vn.first;
        auto neighbors = vn.second;

        for (auto &neigh : neighbors)
        {
            if (neigh.dest->color == color::WHITE)
            {
                neigh.dest->dist = curr.dist + neigh.weight;
                neigh.dest->prev = &curr;
                ++time;
                neigh.dest->turn_gray(time);
                q.push(neigh.dest->name);
            }
        }
        ++time;
        curr.turn_black(time);
        q.pop();
    }
}

