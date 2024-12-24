#include <iostream>
#include <string>
#include "MyVector.h"
using namespace std;

class Tree {
public:

    std::string name;
    std::string value;
    Tree* left;
    Tree* right;
                             // Конструктор
    Tree(const Tree& other);              // Конструктор копирования
    Tree(Tree&& other) noexcept;          // Конструктор перемещения
    Tree& operator=(const Tree& other);   // Оператор копирующего присваивания
    Tree& operator=(Tree&& other) noexcept; // Оператор перемещающего присваивания
   
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
        return this->left;
    }

    Tree* GetRightNode() const {
        return this->right;
    }

    // Функция для печати дерева с использованием символов ASCII
    void PrintASCII(const string& prefix = "", bool isLeft = true) const {
        cout << prefix;

        cout << (isLeft ? "|__" : "|__");

        cout << name << endl;

        // Получение всех детей векторе
        mylib::Vector<Tree*> children;
        if (left) children.push_back(left);
        if (right) children.push_back(right);

        for (size_t i = 0; i < children.size(); ++i) {
            const bool last = (i == children.size() - 1);
            children[i]->PrintASCII(prefix + (isLeft ? "|   " : "    "), !last);
        }
    }

    std::string GetValue() { return this != nullptr ? this->value : ""; }

    // Существующий метод для печати с отступами
    void PrintTreeIndented(int indent = 0) const {
        for (int i = 0; i < indent; ++i) cout << "  ";
        cout << name << endl;
        if (left) left->PrintTreeIndented(indent + 1);
        if (right) right->PrintTreeIndented(indent); // Сиблинг-узлы выводятся на том же уровне
    }

    void FreeTree(Tree* node) {
        if (node == nullptr) return;
        FreeTree(node->left); // Предполагается, что у вас есть указатели на детей
        FreeTree(node->right);
        delete node;
    }

    void PreOrderTraversal(Tree* node) {
        if (!node) return; // Базовый случай: если узел пустой, выходим из функции.

        // Обрабатываем текущий узел
        cout << node->name << " ";

        // Рекурсивно обходим левое поддерево
        PreOrderTraversal(node->left);

        // Рекурсивно обходим правое поддерево
        PreOrderTraversal(node->right);
    }

    Tree() = default;
};
