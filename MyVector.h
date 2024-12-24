#ifndef MY_VECTOR_H
#define MY_VECTOR_H

#include <cstddef> // Для size_t
#include <stdexcept> // Для std::out_of_range

namespace mylib {

    // Итератор для работы с Vector
    template <typename T>
    class Iterator {
    public:
        Iterator() : m_ptr(nullptr) {} // Конструктор по умолчанию инициализирует m_ptr как nullptr


        explicit Iterator(T* ptr) : m_ptr(ptr) {}

        const T* operator->() const { return m_ptr; }
        const T& operator*() const { return *m_ptr; }
        T* operator->() { return m_ptr; } // Перегрузка оператора -> для доступа к членам объекта
        T& operator*() {
            if (!m_ptr) throw std::runtime_error("Dereferencing null iterator");
            return *m_ptr;
        }
        Iterator& operator++() {
            if (m_ptr == nullptr) {
                throw std::runtime_error("Iterator increment on null pointer");
            }
            ++m_ptr;
            return *this;
        }
        Iterator& operator--() noexcept{ m_ptr--; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        Iterator operator--(int) { Iterator tmp = *this; --(*this); return tmp; }
        bool operator==(const Iterator& other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const Iterator& other) const { return m_ptr != other.m_ptr; }

    private:
        T* m_ptr;
    };

    // Реализация динамического массива
    template <typename T>
    class Vector {
    public:
        using Iterator = mylib::Iterator<T>; // Добавляем вложенный тип Iterator

        Vector() : m_data(nullptr), m_size(0), m_capacity(0) {}

        ~Vector() { delete[] m_data; }

        // Конструктор копирования
        Vector(const Vector& other) : m_size(other.m_size), m_capacity(other.m_capacity) {
            m_data = new T[m_capacity];
            for (size_t i = 0; i < m_size; ++i) {
                m_data[i] = other.m_data[i];
            }
        }

        // Оператор присваивания копированием
        Vector& operator=(const Vector& other) {
            if (this != &other) {
                delete[] m_data;
                m_size = other.size;
                m_capacity = other.m_capacity;
                m_data = new T[m_capacity];
                for (size_t i = 0; i < m_size; ++i) {
                    m_data[i] = other.m_data[i];
                }
            }
            return *this;
        }

        void push_back(const T& value) {
            if (m_size == m_capacity) {
                resize();
            }
            m_data[m_size++] = value;
        }

        void clear() {
            for (size_t i = 0; i < m_size; ++i) {
                m_data[i].~T(); // Явный вызов деструктора
            }
            m_size = 0;
        }

        // Константная версия для работы с const объектами
        T& back() {
            if (this == nullptr || m_data == nullptr) {
                throw std::runtime_error("Invalid object or uninitialized data");
            }
            if (m_size == 0) {
                throw std::out_of_range("Vector is empty");
            }
            return m_data[m_size - 1];
        }

        T& at(size_t index) {
            if (index >= m_size) {
                throw std::out_of_range("Index out of range");
            }
            return m_data[index];
        }


        T& operator[](size_t index) { return m_data[index]; }
        const T& operator[](size_t index) const { return m_data[index]; }

        size_t size() const { return m_size; }
        bool empty() const { return m_size == 0; }

        Iterator begin() { return Iterator(m_data); }
        Iterator end() { return Iterator(m_data + m_size); }

    private:
        void resize() {
            m_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            T* new_data = new T[m_capacity];
            for (size_t i = 0; i < m_size; ++i) {
                new_data[i] = m_data[i];
            }
            delete[] m_data;
            m_data = new_data;
        }

        T* m_data;
        size_t m_size;
        size_t m_capacity;
    };

} // namespace mylib

#endif // MY_VECTOR_H
