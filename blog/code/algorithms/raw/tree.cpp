#include "graph.h"
#include <queue>
using namespace std;
using namespace graph;



void test()
{
    string tree = R"(
        digraph A {
            node [margin=0, shape=circle, penwidth=2]
            ordering=out
            NULL_G_LC [shape=point]
            NULL_I_RC [shape=point]
            F -> B
            F -> G
            B -> A
            B -> D
            D -> C
            D -> E
            G -> NULL_G_LC
            G -> I
            I -> H
            I -> NULL_I_RC
        }
    )";
    auto g = graph::interpret(tree);

    g->display();
    tree_bfs(*g, "F");
    ofstream of("bfs.dot");
    export_tree_dotfile(*g, "F", of);

    tree_bfs(*g, "F");
    ofstream of2("bfs2.dot");
    export_tree_dotfile(*g, "F", of2);
    
    {
        tree_preorder(*g, "F");
        ofstream of("preorder.dot");
        export_tree_dotfile(*g, "F", of);
    }

    {
        tree_inorder(*g, "F");
        ofstream of("inorder.dot");
        export_tree_dotfile(*g, "F", of);
    }

    {
        tree_postorder(*g, "F");
        ofstream of("postorder.dot");
        export_tree_dotfile(*g, "F", of);
    }

}

int main(int argc, char **argv)
{
    test();
}
