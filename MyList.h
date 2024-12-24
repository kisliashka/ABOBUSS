#ifndef MY_LIST_H
#define MY_LIST_H
template <typename T>
class MyList {
public:
    // Узел списка
    struct Node {
        T data;
        Node* next;
    };

    MyList() : head(nullptr), tail(nullptr), size_(0) {}

    // Добавление элемента в список
    void push_back(const T& value) {
        Node* new_node = new Node{ value, nullptr };
        if (tail) {
            tail->next = new_node;
        }
        else {
            head = new_node;
        }
        tail = new_node;
        ++size_;
    }

    // Итератор для списка
    class Iterator {
    public:
        Iterator(Node* node) : current(node) {}

        T& operator*() { return current->data; }
        Iterator& operator++() {
            if (current) {
                current = current->next;
            }
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }

    private:
        Node* current;
    };

    // Возвращаем итератор на начало списка
    Iterator begin() { return Iterator(head); }

    // Возвращаем итератор на конец списка
    Iterator end() { return Iterator(nullptr); }

    // Функция получения размера списка
    size_t size() const { return size_; }

private:
    Node* head;
    Node* tail;
    size_t size_; // Переименовали переменную-член
};
#endif // MY_LIST_H
