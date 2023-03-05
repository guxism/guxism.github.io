

#include "graph.h"
#include <queue>
using namespace std;
using namespace graph;

void dijkstra(Graph &g, std::string start);

int main(int argc, char **argv)
{
    string graph = R"(
        digraph A {
            ratio=fill
            splines=spline
            overlap=scale
            rankdir=LR 
            node [margin=0, shape=circle, penwidth=2]
            s -> t[label=10]
            s -> y[label=5]
            t -> y[label=2]
            t -> x[label=1]
            y -> t[label=3]
            y -> z[label=2]
            y -> x[label=9]
            x -> z[label=4]
            z -> x[label=6]
            z -> s[label=7]
        }
    )";
    auto g = graph::interpret(graph);

    g->display();
    dijkstra(*g, "s");
    ofstream of("dijkstra.dot");
    export_graph_w(*g, of);
    ofstream of2("dijkstra_shortest_path.dot");
    find_shortest_path(*g, "s", "x", of2);
}

void dijkstra(Graph &g, std::string start)
{
    g.cleanup_color();
    g.set_all_dist_inf();

    auto cmp = [](Vertex *c, Vertex *p) { return c->dist > p->dist; };
    std::priority_queue<Vertex *, std::vector<Vertex *>, decltype(cmp)> q;
    Vertex &start_vertex = g.get_vertex(start).first;

    unsigned time = 0;

    start_vertex.dist = 0;
    start_vertex.turn_gray(++time);

    for (auto &v : g.adj_list)
    {
        q.push(&v.second.first);
    }


    while (!q.empty())
    {
        auto u = q.top();
        q.pop();

        if (u->color == color::BLACK)
        {
            continue;
        }

        u->turn_black(++time);

        VertexAndNeighbors &vn = g.get_vertex(u->name);
        auto neighbors = vn.second;
        for (auto &neigh : neighbors)
        {
            if (neigh.dest->color != color::BLACK)
            {
                if (neigh.dest->dist > u->dist + neigh.weight)
                {
                    neigh.dest->turn_gray(++time);
                    neigh.dest->dist = u->dist + neigh.weight;
                    neigh.dest->prev = u;
                    q.push(neigh.dest);
                }
            }
        }
    }
}

