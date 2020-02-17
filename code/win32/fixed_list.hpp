#ifndef __FIXED_LIST_HPP__
#define __FIXED_LIST_HPP__

template<typename T>
class ListNode
{
    ListNode<T>* prev;
    ListNode<T>* next;
    T data;
};

template <typename T, typename capacity>
class FixedList
{
public:
    int size;
    ListNode<T> nodes[capacity];
    ListNode<T>* head;
    ListNode<T>* tail;
    ListNode<T>* nextFree;

    FixedList() {
        for (int i = 0; i < capacity-1; i++) {
        }
    }

    T* add() {

    }

    void add(const T& data) {

    }
};










#endif
