#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Tree {
public:
    std::string name;
    Tree* left;
    Tree* right;

    Tree(const std::string& nodeName) : name(nodeName), left(nullptr), right(nullptr) {}

    static Tree* CreateNode(const std::string& nodeName) {
        return new Tree(nodeName);
    }

    void AddLeftTree(Tree* child) {
        left = child;
    }

    void AddRightTree(Tree* sibling) {
        right = sibling;
    }

    Tree* GetLeftNode() const {
        return left;
    }

    Tree* GetRightNode() const {
        return right;
    }

    // Функция для печати дерева с использованием символов ASCII
    void PrintASCII(const string& prefix = "", bool isLeft = true) const {
        cout << prefix;

        cout << (isLeft ? "|__" : "|__");

        cout << name << endl;

        // Получение всех детей векторе
        vector<Tree*> children;
        if (left) children.push_back(left);
        if (right) children.push_back(right);

        for (size_t i = 0; i < children.size(); ++i) {
            bool last = (i == children.size() - 1);
            children[i]->PrintASCII(prefix + (isLeft ? "|   " : "    "), !last);
        }
    }

    // Существующий метод для печати с отступами
    void PrintTreeIndented(int indent = 0) const {
        for (int i = 0; i < indent; ++i) cout << "  ";
        cout << name << endl;
        if (left) left->PrintTreeIndented(indent + 1);
        if (right) right->PrintTreeIndented(indent); // Сиблинг-узлы выводятся на том же уровне
    }

    static void FreeTree(Tree* root) {
        if (!root) return;
        FreeTree(root->left);
        FreeTree(root->right);
        delete root;
    }
};
