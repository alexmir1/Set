#pragma once
#include <algorithm>
#include <cassert>
#include <vector>

template<class ValueType>
struct WrapperWithAdequateOperators {
    ValueType elem;
    WrapperWithAdequateOperators(const ValueType& elem) : elem(elem) {}
    bool operator<(const WrapperWithAdequateOperators& other) const {
        return elem < other.elem;
    }
    bool operator!=(const WrapperWithAdequateOperators& other) const {
        return elem < other.elem || other.elem < elem;
    }
    bool operator==(const WrapperWithAdequateOperators& other) const {
        return !(*this != other);
    }
    bool operator>=(const WrapperWithAdequateOperators& other) const {
        return !(elem < other.elem);
    }
    bool operator>(const WrapperWithAdequateOperators& other) const {
        return *this >= other && *this != other;
    }
    bool operator<=(const WrapperWithAdequateOperators& other) const {
        return !(other.elem < elem);
    }
};

template<class ValueType>
class Set {
    struct node {
        std::vector<WrapperWithAdequateOperators<ValueType>> keys;
        node* parent = nullptr;
        std::vector<node*> childs;

        node(std::vector<WrapperWithAdequateOperators<ValueType>> keys) : keys(keys) {}
        
        node(std::vector<WrapperWithAdequateOperators<ValueType>> keys, node* parent,
            std::vector<node*> childs) : keys(keys), parent(parent), childs(childs) {}

        ~node() {
            for (node* child : childs)
                if (child)
                    delete child;
        }

        void push_key() {
            if (parent) {
                size_t pos = std::find(parent->childs.begin(), parent->childs.end(),
                    this) - parent->childs.begin();
                if (parent->keys[pos] != keys.back()) {
                    parent->keys[pos] = keys.back();
                    if (pos + 1 == parent->childs.size())
                        parent->push_key();
                }
            }
        }
    };
    node* root = nullptr;
    size_t my_size = 0;
public:
    Set() {}

    template<class Iter>
    Set(Iter begin, Iter end) {
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }

    Set(std::initializer_list<ValueType> set) {
        for (const auto & elem : set)
            insert(elem);
    }

    Set(const Set & set) {
        for (const auto & elem : set)
            insert(elem);
    }

    const Set & operator=(const Set & set) {
        if (this != &set) {
            clear();
            for (const auto & elem : set)
                insert(elem);
        }
        return *this;
    }

    ~Set() {
        clear();
    }

    size_t size() const {
        return my_size;
    }

    void clear() {
        if (root) {
            delete root;
            root = nullptr;
            my_size = 0;
        }
    }

    bool empty() const {
        return my_size == 0;
    }

    void insert(const ValueType& inadequate_elem) {
        WrapperWithAdequateOperators<ValueType> elem(inadequate_elem);
        if (root) {
            node* cur = root;
            while (cur->childs.size() != 0) {
                if (elem < cur->keys[0])
                    cur = cur->childs[0];
                else if (elem == cur->keys[0] || (cur->keys.size() > 1 && elem == cur->keys[1]))
                    return;
                else if (cur->keys.size() > 1 && elem < cur->keys[1])
                    cur = cur->childs[1];
                else
                    cur = cur->childs.back();
            }
            if (cur->keys[0] != elem) {
                ++my_size;
                if (cur->parent) {
                    cur = cur->parent;
                    size_t pos = std::lower_bound(cur->keys.begin(), cur->keys.end(), elem) -
                        cur->keys.begin();
                    cur->keys.insert(cur->keys.begin() + pos, elem);
                    cur->childs.insert(cur->childs.begin() + pos, new node({elem}, cur, {}));
                    cur->push_key();
                    while (cur->childs.size() == 4) {
                        node* new_node = new node({cur->keys[0], cur->keys[1]}, cur->parent,
                            {cur->childs[0], cur->childs[1]});
                        cur->childs[0]->parent = cur->childs[1]->parent = new_node;
                        cur->keys.erase(cur->keys.begin(), cur->keys.begin() + 2);
                        cur->childs.erase(cur->childs.begin(), cur->childs.begin() + 2);
                        if (cur->parent) {
                            size_t pos = std::lower_bound(cur->parent->keys.begin(),
                                cur->parent->keys.end(), new_node->keys[0]) - cur->parent->keys.begin();
                            cur->parent->keys.insert(cur->parent->keys.begin() + pos, new_node->keys[1]);
                            cur->parent->childs.insert(cur->parent->childs.begin() + pos, new_node);
                            cur = cur->parent;
                        } else {
                            root = cur->parent = new_node->parent =
                                new node({new_node->keys[1], cur->keys[1]}, nullptr, {new_node, cur});
                        }
                    }
                } else {
                    if (elem > cur->keys[0])
                        std::swap(elem, cur->keys[0]);
                    node* new_node = new node({elem});
                    root = cur->parent = new_node->parent =
                        new node({new_node->keys[0], cur->keys[0]}, nullptr, {new_node, cur});
                }
            }
        } else {
            ++my_size;
            root = new node({elem});
        }
    }

    void erase(const ValueType & inadequate_elem) {
        WrapperWithAdequateOperators<ValueType> elem(inadequate_elem);
        if (root) {
            node* cur = root;
            while (cur->childs.size() != 0) {
                if (elem <= cur->keys[0])
                    cur = cur->childs[0];
                else if (cur->keys.size() > 1 && elem <= cur->keys[1])
                    cur = cur->childs[1];
                else
                    cur = cur->childs.back();
            }
            if (cur->keys[0] == elem) {
                --my_size;
                if (cur->parent) {
                    cur = cur->parent;
                    size_t pos = std::lower_bound(cur->keys.begin(), cur->keys.end(), elem) - cur->keys.begin();
                    cur->keys.erase(cur->keys.begin() + pos);
                    delete cur->childs[pos];
                    cur->childs.erase(cur->childs.begin() + pos);
                    cur->push_key();
                    while (cur->childs.size() < 2) {
                        if (cur->parent) {
                            node* pr = cur;
                            cur = cur->parent;
                            size_t pos = std::find(cur->childs.begin(), cur->childs.end(), pr)
                                - cur->childs.begin();
                            size_t another = pos == 0 ? 1 : pos - 1;
                            if (cur->childs[another]->childs.size() == 2) {
                                if (pos == 0)
                                    std::swap(pos, another);
                                cur->childs[another]->childs.insert(cur->childs[another]->
                                    childs.end(), cur->childs[pos]->childs.begin(), cur->
                                    childs[pos]->childs.end());
                                cur->childs[another]->keys.insert(cur->childs[another]->
                                    keys.end(), cur->childs[pos]->keys.begin(), cur->childs[pos]->
                                    keys.end());
                                for (auto x : cur->childs[pos]->childs)
                                    x->parent = cur->childs[another];
                                cur->childs[pos]->childs.clear();
                                delete cur->childs[pos];
                                cur->childs.erase(cur->childs.begin() + pos);
                                cur->keys.erase(cur->keys.begin() + another);
                            } else {
                                if (pos == 0) {
                                    cur->childs[pos]->childs.push_back(cur->childs[another]->
                                        childs[0]);
                                    cur->childs[another]->childs[0]->parent = cur->childs[pos];
                                    cur->childs[pos]->keys.push_back(cur->childs[another]->
                                        keys[0]);
                                    cur->childs[another]->childs.erase(cur->childs[another]->
                                        childs.begin());
                                    cur->childs[another]->keys.erase(cur->childs[another]->
                                        keys.begin());
                                    cur->keys[pos] = cur->childs[pos]->keys.back();
                                } else {
                                    cur->childs[pos]->childs.insert(cur->childs[pos]->
                                        childs.begin(), cur->childs[another]->childs.back());
                                    cur->childs[another]->childs.back()->parent = cur->childs[pos];
                                    cur->childs[pos]->keys.insert(cur->childs[pos]->
                                        keys.begin(), cur->childs[another]->keys.back());
                                    cur->childs[another]->childs.pop_back();
                                    cur->childs[another]->keys.pop_back();
                                    cur->keys[another] = cur->childs[another]->keys.back();
                                }
                            }
                        } else {
                            root = cur->childs[0];
                            root->parent = nullptr;
                            cur->childs.clear();
                            delete cur;
                            break;
                        }
                    }
                } else {
                    delete root;
                    root = nullptr;
                }
            }
        }
    }

    class iterator {
        node* cur;
        bool end;

    public:
        iterator() : cur(nullptr), end(true) {}

        iterator(node* cur, bool end = false) : cur(cur), end(end) {}
        
        const iterator& operator++() {
            node* p_state = cur;
            node* prev = cur;
            cur = cur->parent;
            while (cur) {
                size_t pos = std::find(cur->childs.begin(), cur->childs.end(), prev) -
                    cur->childs.begin();
                prev = cur;
                if (pos + 1 == cur->childs.size())
                    cur = cur->parent;
                else {
                    cur = cur->childs[pos + 1];
                    break;
                }
            }
            if (cur) {
                while (cur->childs.size()) {
                    cur = cur->childs[0];
                }
            } else {
                cur = p_state;
                end = true;
            }
            return *this;
        }
        
        const iterator& operator--() {
            if (end)
                end = false;
            else {
                node* prev = cur;
                cur = cur->parent;
                while (cur) {
                    size_t pos = std::find(cur->childs.begin(), cur->childs.end(), prev) -
                        cur->childs.begin();
                    prev = cur;
                    if (pos == 0)
                        cur = cur->parent;
                    else {
                        cur = cur->childs[pos - 1];
                        break;
                    }
                }
                while (cur->childs.size()) {
                    cur = cur->childs.back();
                }
            }
            return *this;
        }
        
        iterator operator++(int) {
            auto ret = *this;
            ++*this;
            return ret;
        }
        
        iterator operator--(int) {
            auto ret = *this;
            --*this;
            return ret;
        }
        
        const ValueType& operator*() const {
            return cur->keys[0].elem;
        }
        
        const ValueType* operator->() const {
            return &cur->keys[0].elem;
        }
        
        bool operator==(const iterator& another) const {
            return cur == another.cur && end == another.end;
        }
        
        bool operator!=(const iterator& another) const {
            return cur != another.cur || end != another.end;
        }
    };

    iterator begin() const {
        if (root) {
            node* cur = root;
            while (cur->childs.size()) {
                cur = cur->childs[0];
            }
            return {cur};
        } else {
            return {nullptr, true};
        }
    }
    
    iterator end() const {
        if (root) {
            node* cur = root;
            while (cur->childs.size()) {
                cur = cur->childs.back();
            }
            return {cur, true};
        } else {
            return {nullptr, true};
        }
    }
    
    iterator find(const ValueType & inadequate_elem) const {
        WrapperWithAdequateOperators<ValueType> elem(inadequate_elem);
        if (root) {
            node* cur = root;
            while (cur->childs.size() != 0) {
                if (elem <= cur->keys[0])
                    cur = cur->childs[0];
                else if (cur->keys.size() > 1 && elem <= cur->keys[1])
                    cur = cur->childs[1];
                else
                    cur = cur->childs.back();
            }
            if (cur->keys[0] == elem)
                return {cur};
            else
                return end();
        } else {
            return end();
        }
    }
    
    iterator lower_bound(const ValueType & inadequate_elem) const {
        WrapperWithAdequateOperators<ValueType> elem(inadequate_elem);
        if (root) {
            node* cur = root;
            while (cur->childs.size() != 0) {
                size_t pos = std::lower_bound(cur->keys.begin(), cur->keys.end(), elem) -
                    cur->keys.begin();
                if (pos == cur->keys.size())
                    return end();
                cur = cur->childs[pos];
            }
            if (cur->keys[0] >= elem)
                return {cur};
            else
                return end();
        } else {
            return end();
        }
    }
};
