
#ifndef GRAPH2
#define GRAPH2

#include "peglib.h"
#include <any>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace peg::udl;

namespace graph
{

using KV = std::pair<std::string, std::string>;
using Map = std::map<std::string, std::string>;

enum color
{
    WHITE,
    GREY,
    BLACK,
};

struct Attrs
{
    std::string name;
    Map attrs;
};

struct Vertex
{
    Vertex *prev;
    std::string name;
    int dist;
    enum color color;
    unsigned int gray_time;
    unsigned int black_time;
    enum color pcolor;
    Vertex()
        : prev(nullptr), name(""), dist(0), color(color::WHITE), gray_time(0),
          black_time(0), pcolor(color::WHITE)
    {
    }
    Vertex(std::string &n)
        : prev(nullptr), name(n), dist(0), color(color::WHITE), gray_time(0),
          black_time(0), pcolor(color::WHITE)
    {
    }
    Vertex(std::string &n, int w)
        : prev(nullptr), name(n), dist(w), color(color::WHITE), gray_time(0),
          black_time(0), pcolor(color::WHITE)
    {
    }

    void turn_gray(unsigned int time)
    {
        color = color::GREY;
        gray_time = time;
    }

    void turn_black(unsigned int time)
    {
        color = color::BLACK;
        black_time = time;
    }
};

struct Edge
{
    Vertex *src;
    Vertex *dest;
    int weight;
    Edge() : src(nullptr), dest(nullptr), weight(0) {}
};

using VertexAndNeighbors = std::pair<Vertex, std::vector<Edge>>;
struct Entry;
struct Graph
{
    std::map<std::string, std::pair<Vertex, std::vector<Edge>>> adj_list;
    std::vector<KV> graph_attrs;
    std::string type;
    std::vector<Attrs> node_attrs;
    std::vector<Attrs> node_decls;
    std::vector<Entry> entries;
    void display() const
    {
        for (auto p : adj_list)
        {
            std::cout << p.first << " ";
            for (auto e : p.second.second)
            {
                std::cout << e.dest->name << ":" << e.weight << " ";
            }
            std::cout << std::endl;
        }
    }

    Vertex *insert_vertex(Vertex &&v)
    {
        if (adj_list.find(v.name) == adj_list.end())
        {
            std::pair<Vertex, std::vector<Edge>> p(v, {});
            adj_list[v.name] = std::move(p);
        }
        return &(adj_list[v.name].first);
    }

    VertexAndNeighbors &get_vertex(std::string n) { return adj_list[n]; }

    void insert_edge(Edge &&e)
    {
        if (adj_list.find(e.src->name) != adj_list.end())
        {
            adj_list[e.src->name].second.push_back(e);
        }
    }

    std::vector<std::string> iter_facit(std::string start, std::string end)
    {
        Vertex *curr = &adj_list[end].first;
        std::vector<std::string> iter;
        do
        {
            iter.push_back(curr->name);
            if (curr->name == start)
            {
                break;
            }
            curr = curr->prev;
        } while (curr);
        for (auto ri = iter.rbegin(); ri != iter.rend(); ++ri)
        {
            std::cout << *ri << " ";
        }
        std::cout << std::endl;
        return iter;
    }

    static std::string compose_new_vertex_name(Vertex *v, int print_time = 0)
    {
        std::string quote = "\"";
        std::ostringstream os;
        os << quote << v->name;
        if (!print_time)
        {
            os << "(" << v->gray_time << ":" << v->black_time << ")";
        }
        os << quote;
        return os.str();
    }

    static std::string compose_new_vertex_name(Graph &g, std::string &name,
                                               int print_time = 0)
    {
        std::string quote = "\"";
        std::ostringstream os;
        os << quote << name;
        Vertex &v = (g.adj_list)[name].first;
        if (!print_time)
        {
            os << "(" << v.gray_time << ":" << v.black_time << ")";
        }
        os << quote;
        return os.str();
    }

    static std::string compose_new_vertex_name_w(Graph &g, std::string &name,
                                                 int print_time = 0)
    {
        std::string quote = "\"";
        std::ostringstream os;
        os << quote << name;
        Vertex &v = (g.adj_list)[name].first;
        if (!print_time)
        {
            os << "[" << v.dist << "]";
            os << "(" << v.gray_time << ":" << v.black_time << ")";
        }
        os << quote;
        return os.str();
    }

    static std::string compose_node_attrs(std::vector<Attrs> &s)
    {
        std::ostringstream out;
        for (auto &a : s)
        {
            out << "\tnode [";
            auto i = a.attrs.begin();
            for (; i != std::prev(a.attrs.end()); ++i)
            {
                out << i->first << "=" << i->second << ", ";
            }
            out << i->first << "=" << i->second;
            out << "]" << std::endl;
        }
        return out.str();
    }

    static std::string compose_node_decls(std::vector<Attrs> &s)
    {
        std::ostringstream out;
        for (auto &a : s)
        {
            out << "\t"
                << "\"" << a.name << "\"";
            if (!a.attrs.empty())
            {
                out << "[" << std::endl;
                auto i = a.attrs.begin();
                for (; i != std::prev(a.attrs.end()); ++i)
                {
                    out << i->first << "=" << i->second << ", ";
                }
                out << i->first << "=" << i->second;
                out << "]";
            }
            out << std::endl;
        }
        return out.str();
    }

    static std::string compose_graph_attrs(std::vector<KV> &s)
    {
        std::ostringstream out;
        for (auto &a : s)
        {
            out << "\t" << a.first << "=" << a.second << std::endl;
        }
        return out.str();
    }

    void cleanup_color()
    {
        for (auto &vn : adj_list)
        {
            vn.second.first.color = color::WHITE;
            vn.second.first.pcolor = color::WHITE;
        }
    }

    void set_all_dist_inf()
    {
        for (auto &vn : adj_list)
        {
            vn.second.first.dist = std::numeric_limits<int>::max();
        }
    }
};

struct NodeStrPair
{
    std::string src;
    std::string dest;
    std::string conn;
    int weight;
};

struct Entry
{
    Entry() : tag(0) {}
    Entry(NodeStrPair n, unsigned int t) : np_str(n), tag(t) {}
    NodeStrPair np_str;
    unsigned int tag;
};

struct Value
{
    std::variant<std::nullptr_t, std::string, bool, long, Map, Entry, Graph, KV,
                 NodeStrPair, Attrs>
        v;

    Value() : v(nullptr) {}

    explicit Value(bool b) : v(b) {}
    explicit Value(long l) : v(l) {}
    explicit Value(std::string s) : v(s) {}
    explicit Value(Graph g) : v(std::move(g)) {}
    explicit Value(Entry e) : v(std::move(e)) {}
    explicit Value(NodeStrPair p) : v(std::move(p)) {}
    explicit Value(KV kv) : v(std::move(kv)) {}
    explicit Value(Map a) : v(std::move(a)) {}
    explicit Value(Attrs a) : v(std::move(a)) {}

    template <typename T> T get() const
    {
        try
        {
            return std::get<T>(v);
        }
        catch (std::bad_variant_access &)
        {
            throw std::runtime_error("type error.");
        }
    }
};

static std::string_view GRAMMAR = R"(

DotFile <- Spacing Graph EndOfFile
Graph <- Keyword Identifier Body
Body <- LPar Entry* RPar
Entry <-  Node2Node / NodeAttr / GraphAttr / NodeDeclaration

NodeAttr <- 'node' Spacing LBox KVs RBox
GraphAttr <- GraphAttr1 / GraphAttr2
GraphAttr1 <- 'graph' Spacing LBox KVs RBox
GraphAttr2 <- Pair

NodeDeclaration <- Identifier (LBox KVs RBox)*

Node2Node <- Identifier Connector Identifier (EdgeAttrs)?

Connector <- (Line / Arrow) Spacing
EdgeAttrs <- LBox KVs RBox

KVs <- Pair (AttrDelim Pair)*

Pair <- Keyword EQ Value Spacing
Value <- Char+

Identifier <- Letter Char* Spacing

Char      <- [a-zA-Z_0-9]
Letter    <- [a-zA-Z_]
LBox      <- '['      Spacing
RBox      <- ']'      Spacing
LPar      <- '{'      Spacing
RPar      <- '}'      Spacing
EQ        <- '='      Spacing
Line      <- '--'
Arrow     <- '->'
AttrDelim <- ',' Spacing

Keyword  <- Reserved (!Char) Spacing
Reserved <- ('label' / 'color' / 'margin' / 'shape' / 'penwidth' / 'graph' / 'rankdir' / 'digraph' / 'ordering' / 'ratio' / 'splines' / 'overlap')

Spacing <- Space*
Space <- ' ' / '\t' / EndOfLine
EndOfLine <- '\r\n' / '\n' / '\r'
EndOfFile <- !.


)";

std::unique_ptr<Graph> interpret(const std::string &source);

struct Interpreter
{
    enum GraphType
    {
        undefined,
        digraph,
        graph
    };
    GraphType type;
    Value eval_char(const peg::Ast &ast, size_t start, size_t end)
    {
        std::string s;
        for (size_t i = start; i < end; i += 1)
        {
            s += (*ast.nodes[i]).token;
        }
        return Value(s);
    }

    Value eval_value(const peg::Ast &ast)
    {
        std::string s;

        for (size_t i = 0; i < ast.nodes.size(); i += 1)
        {
            s += (*ast.nodes[i]).token;
        }
        return Value(s);
    }

    Value eval_identifier(const peg::Ast &ast)
    {
        auto s = std::string((*ast.nodes[0]).token);
        for (size_t i = 1; i < ast.nodes.size() - 1; i += 1)
        {
            s += (*ast.nodes[i]).token;
        }
        return Value(s);
    }

    std::string eval_connector(const peg::Ast &ast)
    {
        return std::string((*ast.nodes[0]).token);
    }

#define eval_keyword eval_identifier

    Value eval_pair(const peg::Ast &ast)
    {
        auto key = eval_keyword(*ast.nodes[0]).get<std::string>();
        auto value = eval_value(*ast.nodes[2]).get<std::string>();
        return Value(KV{key, value});
    }
    Map eval_kvs(const peg::Ast &ast)
    {
        Map map;
        for (size_t i = 0; i < ast.nodes.size(); i += 2)
        {
            auto p = eval_pair(*ast.nodes[i]).get<KV>();
            map.insert(p);
        }

        return map;
    }

    Map eval_nodeattrs(const peg::Ast &ast) { return eval_kvs(*ast.nodes[1]); }

    Value eval_node2node(const peg::Ast &ast)
    {
        NodeStrPair p;
        p.src = eval_identifier(*ast.nodes[0]).get<std::string>();
        p.dest = eval_identifier(*ast.nodes[2]).get<std::string>();
        auto conn = eval_connector(*ast.nodes[1]);
        if (ast.nodes.size() == 4)
        {
            auto map = eval_nodeattrs(*ast.nodes[3]);
            std::string w = std::string(map["label"]);
            p.weight = atoi(w.c_str());
        }
        else
        {
            p.weight = 1;
        }
        p.conn = conn;

        return Value(Entry{p, "Node2Node"_});
    }

    Attrs eval_nodedecl(const peg::Ast &ast)
    {
        Attrs d;
        d.name = eval_identifier(*ast.nodes[0]).get<std::string>();
        if (ast.nodes.size() > 1)
        {
            d.attrs = eval_kvs(*ast.nodes[2]);
        }
        return d;
    }

    Value eval_subentry(const peg::Ast &ast)
    {
        if ((*ast.nodes[0]).tag == "Node2Node"_)
        {
            return eval_node2node(*ast.nodes[0]);
        }
        else if ((*ast.nodes[0]).tag == "NodeDeclaration"_)
        {
            return Value(eval_nodedecl(*ast.nodes[0]));
        }
        else if ((*ast.nodes[0]).tag == "NodeAttr"_)
        {
            return Value(eval_nodedecl(*ast.nodes[0]));
        }
        else
        {
            return Value(Entry());
        }
    }

    Value eval_entry(const peg::Ast &ast) { return eval_subentry(ast); }

    bool direction_assert(Entry &entry)
    {
        if (type == GraphType::digraph)
        {
            if (entry.np_str.conn == "->")
                return true;
        }
        else if (type == GraphType::graph)
        {
            if (entry.np_str.conn == "--")
                return true;
        }
        return false;
    }

    void insert_helper(std::unique_ptr<Graph> &g, Entry &entry)
    {
        Vertex s(entry.np_str.src);
        Vertex d(entry.np_str.dest);
        Edge e;
        auto src = g->insert_vertex(std::move(s));
        auto dest = g->insert_vertex(std::move(d));
        e.src = src;
        e.dest = dest;
        e.weight = entry.np_str.weight;
        g->insert_edge(std::move(e));
    }

    std::unique_ptr<Graph> eval_body(const peg::Ast &ast)
    {
        std::unique_ptr<Graph> g = std::make_unique<Graph>();
        if (type == GraphType::graph)
        {
            g->type = "graph";
        }
        else if (type == GraphType::digraph)
        {
            g->type = "digraph";
        }
        for (size_t i = 1; i < ast.nodes.size() - 1; i += 1)
        {
            auto tag = ast.nodes[i]->nodes[0]->tag;
            if (tag == "Node2Node"_)
            {
                auto entry = eval_entry(*ast.nodes[i]).get<Entry>();
                if (!direction_assert(entry))
                {
                    return nullptr;
                }
                insert_helper(g, entry);
                g->entries.push_back(entry);
                if (entry.np_str.conn == "--")
                {
                    std::swap(entry.np_str.src, entry.np_str.dest);
                    insert_helper(g, entry);
                }
            }
            else if (tag == "NodeDeclaration"_)
            {
                auto d = eval_entry(*ast.nodes[i]).get<Attrs>();
                g->node_decls.push_back(std::move(d));
            }
            else if (tag == "GraphAttr"_)
            {
                if (ast.nodes[i]->nodes[0]->nodes[0]->tag == "GraphAttr2"_)
                {
                    auto d =
                        eval_pair(*ast.nodes[i]->nodes[0]->nodes[0]->nodes[0])
                            .get<KV>();
                    g->graph_attrs.push_back(std::move(d));
                }
                else
                {
                    auto d = eval_pair(*ast.nodes[i]
                                            ->nodes[0]
                                            ->nodes[0]
                                            ->nodes[0]
                                            ->nodes[1])
                                 .get<KV>();
                    g->graph_attrs.push_back(std::move(d));
                }
            }
            else if (tag == "NodeAttr"_)
            {
                auto a = eval_entry(*ast.nodes[i]).get<Attrs>();
                g->node_attrs.push_back(std::move(a));
            }
        }
        return g;
    }

    std::unique_ptr<Graph> eval_graph(const peg::Ast &ast)
    {
        auto kw = eval_keyword(*ast.nodes[0]).get<std::string>();
        if (kw == "graph")
        {
            type = GraphType::graph;
        }
        else if (kw == "digraph")
        {
            type = GraphType::digraph;
        }
        else
        {
            type = GraphType::undefined;
            return nullptr;
        }
        return eval_body(*ast.nodes[2]);
    }

    std::unique_ptr<Graph> eval_dotfile(const peg::Ast &ast)
    {
        return eval_graph(*ast.nodes[1]);
    }
};

std::unique_ptr<Graph> interpret(const std::string &source)
{
    peg::parser parser(graph::GRAMMAR);
    parser.enable_ast();
    std::shared_ptr<peg::Ast> ast;
    parser.parse_n(source.data(), source.size(), ast);
    Interpreter i;
    return i.eval_dotfile(*ast);
}
void export_tree_dotfile(Graph &g, std::string start, std::ostream &out)
{
    std::string conn = " -- ";
    if (g.type == "digraph")
    {
        conn = " -> ";
    }
    out << g.type << " A {" << std::endl;

    std::queue<std::string> q;
    Vertex &start_vertex = g.get_vertex(start).first;
    q.push(start);
    start_vertex.pcolor = color::GREY;

    out << Graph::compose_graph_attrs(g.graph_attrs);
    out << Graph::compose_node_attrs(g.node_attrs);
    out << Graph::compose_node_decls(g.node_decls);

    while (!q.empty())
    {
        std::string name = q.front();
        VertexAndNeighbors &vn = g.get_vertex(name);
        auto &curr = vn.first;
        auto neighbors = vn.second;

        for (auto &neigh : neighbors)
        {
            if (neigh.dest->pcolor == color::WHITE)
            {
                neigh.dest->dist = curr.dist + neigh.weight;
                neigh.dest->prev = &curr;
                neigh.dest->pcolor = color::GREY;
                out << "\t" << Graph::compose_new_vertex_name(&curr) << conn;
                if (neigh.dest->name.starts_with("NULL_"))
                {
                    out << Graph::compose_new_vertex_name(neigh.dest, 1)
                        << std::endl;
                }
                else
                {
                    out << Graph::compose_new_vertex_name(neigh.dest)
                        << std::endl;
                }
                q.push(neigh.dest->name);
            }
        }
        curr.pcolor = color::BLACK;
        q.pop();
    }
    out << " } " << std::endl;
}

void tree_bfs(Graph &g, std::string start)
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
                if (neigh.dest->name.starts_with("NULL_"))
                {
                    continue;
                }
                neigh.dest->dist = curr.dist + neigh.weight;
                neigh.dest->prev = &curr;
                ++time;
                neigh.dest->turn_gray(time);
                std::cout << start << "->" << neigh.dest->name << ": "
                          << neigh.dest->dist << std::endl;
                q.push(neigh.dest->name);
            }
        }
        ++time;
        curr.turn_black(time);
        q.pop();
    }
}

void tree_preorder(Graph &g, std::string start)
{
    g.cleanup_color();
    std::vector<std::string> stack;
    Vertex &start_vertex = g.get_vertex(start).first;
    unsigned time = 0;

    ++time;
    start_vertex.turn_gray(time);
    stack.push_back(start);

    while (!stack.empty())
    {
        std::string name = stack.back();
        VertexAndNeighbors &vn = g.get_vertex(name);
        auto &curr = vn.first;
        auto neighbors = vn.second;
        ++time;
        curr.turn_black(time);
        stack.pop_back();

        for (auto neigh{neighbors.rbegin()}; neigh != neighbors.rend(); ++neigh)
        {
            if (neigh->dest->color == color::WHITE)
            {
                if (neigh->dest->name.starts_with("NULL_"))
                {
                    continue;
                }
                neigh->dest->dist = curr.dist + neigh->weight;
                neigh->dest->prev = &curr;
                ++time;
                neigh->dest->turn_gray(time);
                std::cout << start << "->" << neigh->dest->name << ": "
                          << neigh->dest->dist << std::endl;
                stack.push_back(neigh->dest->name);
            }
        }
    }
}
void tree_inorder(Graph &g, std::string start)
{
    g.cleanup_color();

    std::vector<std::string> stack;
    Vertex &start_vertex = g.get_vertex(start).first;
    unsigned time = 0;

    ++time;
    start_vertex.turn_gray(time);
    stack.push_back(start);


    while (!stack.empty())
    {
        std::string name = stack.back();
        VertexAndNeighbors &vn = g.get_vertex(name);
        auto &curr = vn.first;
        auto neighbors = vn.second;

        if (curr.color == color::BLACK)
        {
            stack.pop_back();
            continue;
        }

        if (
            neighbors.size() == 0 ||
            (neighbors[0].dest->name.starts_with("NULL_") &&
             neighbors[1].dest->name.starts_with("NULL_")))
        {
            ++time;
            curr.turn_black(time);
            stack.pop_back();
            continue;
        }

        if (neighbors[0].dest->color == color::WHITE &&
            !neighbors[0].dest->name.starts_with("NULL_"))
        {
            ++time;
            neighbors[0].dest->turn_gray(time);
            stack.push_back(neighbors[0].dest->name);
            continue;
        }

        ++time;
        curr.turn_black(time);
        ++time;
        neighbors[1].dest->turn_gray(time);
        stack.push_back(neighbors[1].dest->name);
    }
}

void tree_postorder(Graph &g, std::string start)
{
    g.cleanup_color();

    std::vector<std::string> stack;
    Vertex &start_vertex = g.get_vertex(start).first;
    unsigned time = 0;

    ++time;
    start_vertex.turn_gray(time);
    stack.push_back(start);

    while (!stack.empty())
    {
        std::string name = stack.back();
        VertexAndNeighbors &vn = g.get_vertex(name);
        auto &curr = vn.first;
        auto neighbors = vn.second;

        if (neighbors.size() == 0 ||
            (neighbors[0].dest->name.starts_with("NULL_") &&
             neighbors[1].dest->name.starts_with("NULL_")))
        {
            ++time;
            curr.turn_black(time);
            stack.pop_back();
            continue;
        }

        if (neighbors[0].dest->color == color::WHITE &&
            !neighbors[0].dest->name.starts_with("NULL_"))
        {
            ++time;
            neighbors[0].dest->turn_gray(time);
            stack.push_back(neighbors[0].dest->name);
            continue;
        }

        if (neighbors[1].dest->color == color::WHITE &&
            !neighbors[1].dest->name.starts_with("NULL_"))
        {
            ++time;
            neighbors[1].dest->turn_gray(time);
            stack.push_back(neighbors[1].dest->name);
        }
        else
        {
            ++time;
            curr.turn_black(time);
            stack.pop_back();
        }
    }
}

void export_graph(Graph &g, std::string start, std::ostream &out)
{
    std::string conn = " -- ";
    if (g.type == "digraph")
    {
        conn = " -> ";
    }
    out << g.type << " A {" << std::endl;

    std::queue<std::string> q;
    Vertex &start_vertex = g.get_vertex(start).first;
    q.push(start);
    start_vertex.pcolor = color::GREY;

    out << Graph::compose_graph_attrs(g.graph_attrs);
    out << Graph::compose_node_attrs(g.node_attrs);
    out << Graph::compose_node_decls(g.node_decls);

    while (!q.empty())
    {
        std::string name = q.front();
        VertexAndNeighbors &vn = g.get_vertex(name);
        auto &curr = vn.first;
        auto neighbors = vn.second;

        for (auto &neigh : neighbors)
        {
            if (neigh.dest->pcolor == color::WHITE)
            {
                neigh.dest->dist = curr.dist + neigh.weight;
                neigh.dest->prev = &curr;
                neigh.dest->pcolor = color::GREY;
                out << "\t" << Graph::compose_new_vertex_name(&curr) << conn;
                out << Graph::compose_new_vertex_name(neigh.dest) << std::endl;
                q.push(neigh.dest->name);
            }
        }
        curr.pcolor = color::BLACK;
        q.pop();
    }
    out << " } " << std::endl;
}

void export_graph2(Graph &g, std::ostream &out)
{
    std::string conn = " -- ";
    if (g.type == "digraph")
    {
        conn = " -> ";
    }
    out << g.type << " A {" << std::endl;

    out << Graph::compose_graph_attrs(g.graph_attrs);
    out << Graph::compose_node_attrs(g.node_attrs);
    out << Graph::compose_node_decls(g.node_decls);

    for (auto &entry : g.entries)
    {
        out << "\t" << Graph::compose_new_vertex_name(g, entry.np_str.src)
            << conn;
        out << Graph::compose_new_vertex_name(g, entry.np_str.dest)
            << std::endl;
    }
    out << " } " << std::endl;
}

void export_graph_w(Graph &g, std::ostream &out)
{
    std::string conn = " -- ";
    if (g.type == "digraph")
    {
        conn = " -> ";
    }
    out << g.type << " A {" << std::endl;

    out << Graph::compose_graph_attrs(g.graph_attrs);
    out << Graph::compose_node_attrs(g.node_attrs);
    out << Graph::compose_node_decls(g.node_decls);

    for (auto &entry : g.entries)
    {
        out << "\t" << Graph::compose_new_vertex_name_w(g, entry.np_str.src)
            << conn;
        out << Graph::compose_new_vertex_name_w(g, entry.np_str.dest);
        out << "[label=" << entry.np_str.weight << "]";
        out << std::endl;
    }
    out << " } " << std::endl;
}

struct ResultGraph
{
    std::string start;
    Graph &g;
};
void find_shortest_path(Graph &g, std::string start, std::string end,
                        std::ostream &out)
{
    std::string conn = " -- ";
    if (g.type == "digraph")
    {
        conn = " -> ";
    }
    out << g.type << " A {" << std::endl;

    out << Graph::compose_graph_attrs(g.graph_attrs);
    out << Graph::compose_node_attrs(g.node_attrs);
    out << Graph::compose_node_decls(g.node_decls);

    auto end_vertex = g.get_vertex(end);
    Vertex *curr = &end_vertex.first;
    std::map<std::string, std::string> should_be_colored;
    while (curr->prev)
    {
        should_be_colored[curr->prev->name] = curr->name;
        curr = curr->prev;
    }

    for (auto &entry : g.entries)
    {
        out << "\t" << Graph::compose_new_vertex_name_w(g, entry.np_str.src)
            << conn;
        out << Graph::compose_new_vertex_name_w(g, entry.np_str.dest);
        out << "[label=" << entry.np_str.weight;
        auto i = should_be_colored.end();
        if ((i = should_be_colored.find(entry.np_str.src)) !=
                should_be_colored.end() &&
            i->second == entry.np_str.dest)
        {
            out << ", color=red, penwidth=2";
        }

        out << "]";
        out << std::endl;
    }
    out << " } " << std::endl;
}

} // namespace graph

#endif