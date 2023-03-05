#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <queue>
#include <sstream>
#include <stack>
#include <string>

enum class Color
{
    red,
    black
};
template <typename T> struct TreeNode;
template <typename T> struct Tree
{
    Tree() : root(nullptr) {}
    using Node = TreeNode<T>;
    TreeNode<T> *root;

    ~Tree()
    {
        std::stack<Node *> s;
        Node *node = this->root;
        while (!s.empty() || node)
        {
            if (node)
            {
                s.push(node);
                node = node->left;
            }
            else
            {
                node = s.top();
                if (node->right)
                {
                    node = node->right;
                }
                else
                {
                    if (node->parent)
                    {
                        if (node == node->parent->left)
                        {
                            node->parent->left = nullptr;
                        }
                        else
                        {
                            node->parent->right = nullptr;
                        }
                    }
                    s.pop();
                    delete node;
                    node = nullptr;
                }
            }
        }
    }

    Node *insert_at(Node *node, T &v)
    {
        Node **side = v < node->value ? &(node->left) : &(node->right);
        if (*side)
        {
            return insert_at(*side, v);
        }
        else
        {
            Node *t = new Node(v);
            t->parent = node;
            *side = t;
            return t;
        }
    }

    template <typename Arg> TreeNode<T> *insert(Arg &&v)
    {
        if (this->root)
        {
            return this->insert_at(this->root, v);
        }
        else
        {
            this->root = new Node(v);
            return this->root;
        }
    }

    static Node *tree_minimum(Node* x)
    {
        auto curr = x;
        while (curr->left)
        {
            curr = curr->left;
        }
        return curr;
    }

    static Node *tree_maximum(Node *x)
    {
        auto curr = x;
        while (curr->right)
        {
            curr = curr->right;
        }
        return curr;
    }

    template <typename U> Node *find(U &&t)
    { 
        
        Node* node = this->root;
        while (node)
        {
            if (t < node->value)
            {
                node = node->left;
            }
            else if (t > node->value)
            {
                node = node->right;
            }
            else
            {
                return node;
            }
        }
        return nullptr;        
    }



    void transplant(Node *u, Node *v)
    {
        if (!u->parent)
        {
            this->root = v;
        }
        else if (u == u->parent->left)
        {
            u->parent->left = v;
        }
        else
        {
            u->parent->right = v;
        }
        if (v->parent)
        {
            v->parent = u->parent;
        }
    }

    void del(Node *z)
    {
        if (z->left == nullptr)
        {
            transplant(z, z->right);
            delete z;
        }
        else
        {
            if (z->right == nullptr)
            {
                transplant(z, z->left);
                delete z;
            }
            else
            {
                Node *y = Tree::tree_minimum(z->right);
                if (y->parent != z)
                {
                    transplant(y, y->right);
                    y->right = z->right;
                    y->right->parent = y;
                }
                transplant(z, y);
                y->left = z->left;
                y->left->parent = y;
                delete z;
            }
        }
    }

    std::vector<T> to_vector()
    {
        std::vector<T> res;
        std::queue<Node *> q;
        q.push(this->root);

        while (!q.empty())
        {
            Node *curr = q.front();
            q.pop();
            res.push_back(curr->value);
            if (curr->left)
            {
                q.push(curr->left);
            }
            if (curr->right)
            {
                q.push(curr->right);
            }
        }
        return res;
    }

    std::string to_graphviz()
    {
        std::ostringstream os;
        std::queue<Node *> q;
        std::string tab = "\t";
        os << "digraph {" << std::endl;
        os << tab << "node [margin=0, shape=circle, penwidth=2]" << std::endl;
        os << tab << "ordering=out" << std::endl;
        q.push(this->root);
        int null_node_num = 0;
        while (!q.empty())
        {
            Node *curr = q.front();
            q.pop();

            if (curr->parent)
            {
                std::cout << curr->value << "'s parent is "
                          << curr->parent->value << std::endl;
            }

            os << tab << curr->value << "[";
            if (curr->color == Color::red)
            {
                os << "color=red, fillcolor=red, ";
            }
            else
            {
                os << "color=black, fillcolor=black, ";
            }

            os << "fontcolor=white, style=filled]" << std::endl;

            if (curr->left)
            {
                os << tab << curr->value << "->" << curr->left->value
                   << std::endl;
                q.push(curr->left);
            }
            else
            {
                std::string null_node("NULL_");
                null_node += std::to_string(null_node_num++);
                os << tab << null_node << " [shape=point]" << std::endl;
                os << tab << curr->value << "->" << null_node << std::endl;
            }
            if (curr->right)
            {
                os << tab << curr->value << "->" << curr->right->value
                   << std::endl;
                q.push(curr->right);
            }
            else
            {
                std::string null_node("NULL_");
                null_node += std::to_string(null_node_num++);
                os << tab << null_node << " [shape=point]" << std::endl;
                os << tab << curr->value << "->" << null_node << std::endl;
            }
        }

        os << "}" << std::endl;
        return os.str();
    }
    void snapshot(int n, std::string prefix)
    {
        std::string name = prefix + "_" + std::to_string(n);
        name += ".dot";
        std::ofstream of(name);
        of << this->to_graphviz();
    }
    static void left_rotate(Node* x)
    {
        Node *y = x->right;
        if (!y)
            return;
        Node *x_parent = x->parent;
        Node *beta = x->left;

        x->parent = y;
        y->left = x;
        y->parent = x_parent;
        x->right = beta;
        if (beta)
            beta->parent = x;
        if (x_parent->left == x)
            x_parent->left = y;
        else
            x_parent->right = y;
    }

    static void right_rotate(Node *x)
    {
        Node *y = x->parent;
        if (!y)
            return;
        Node *y_parent = y->parent;
        Node *beta = x->right;

        x->right = y;
        y->parent = x;
        x->parent = y_parent;
        y->left = beta;
        if (beta)
            beta->parent = y;
        if (y_parent->left == y)
            y_parent->left = x;
        else
            y_parent->right = x;
    }
    /*

    template <typename Arg> Node *insert(Arg &&v)
    {
        auto curr = this->root;
        Node *parent = nullptr;
        while (curr)
        {
            parent = curr;
            if (v < curr->value)
            {
                curr = curr->left;
            }
            else
            {
                curr = curr->right;
            }
        }

        Node *new_node = new Node(v, Color::red);
        new_tree->parent = parent;

        if (parent)
        {
            if (v < parent->value)
            {
                parent->left = new_tree;
            }
            else
            {
                parent->right = new_tree;
            }
        }
        insert_fixup(new_node);
        return new_node;
    }
    */
};


template <typename T> struct TreeNode
{

    template <typename U>
    TreeNode(U &&v, Color c = Color::black)
        : value(v), color(c), left(nullptr), right(nullptr), parent(nullptr)
    {
    }

    ~TreeNode() {}
    T value;
    Color color;
    TreeNode *left, *right, *parent;





    template <typename Arg> TreeNode *insert_fixup(Arg &&v) {}


};

TEST(Tree, basic_binary_tree)
{
    Tree<int> tree;
    tree.insert(5);
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);
    tree.insert(4);
    tree.insert(9);
    tree.insert(6);
    tree.insert(7);
    tree.insert(8);
    tree.snapshot(0, "tree");
    auto a = tree.find(6);
    tree.del(a);
    tree.snapshot(1, "tree");
    a = tree.find(5);
    tree.del(a);
    tree.snapshot(2, "tree");
    auto vec1 = tree.to_vector();
    decltype(vec1) vec2 {7, 1, 9, 2, 8, 3, 4};
    EXPECT_EQ(vec1, vec2);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}