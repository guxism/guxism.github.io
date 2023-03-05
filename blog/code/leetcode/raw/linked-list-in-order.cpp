#include <iostream>
#include <vector>

using namespace std;

struct Node
{
    int value;
    Node *next;
    Node(int i) : value(i), next(nullptr) {}
};

struct Link
{
    Node *head;
    Link() { head = new Node(-1); }
    ~Link()
    {
        auto curr = head;
        while (curr)
        {
            auto temp = curr->next;
            delete curr;
            curr = temp;
        }
    }
    void add(int val)
    {
        auto curr = head;
        auto new_node = new Node(val);
        bool done = false;
        while (curr->next)
        {
            if (curr->next->value < val) {
                curr = curr->next;
            }
            else {
                auto temp = curr->next;
                curr->next = new_node;
                new_node->next = temp;
                done = true;
                break;
            }
        }
        if (!done)
            curr->next = new_node;
    }

    void show()
    {
        auto curr = head->next;
        while (curr)
        {
            cout << curr->value << " ";
            curr = curr->next;
        }
        cout << endl;
    }
};


int main()
{
    Link link;
    link.add(8);
    link.show();
    link.add(3);
    link.show();
    link.add(5);
    link.show();
    link.add(8);
    link.show();
    link.add(2);
    link.show();
    link.add(9);
    link.show();
    link.add(1);
    link.show();
}

