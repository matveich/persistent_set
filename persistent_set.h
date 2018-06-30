//
// Created by matveich on 29.06.18.
//

#ifndef PERSISTENT_SET_PERSISTENT_SET_H
#define PERSISTENT_SET_PERSISTENT_SET_H

#include <vector>
#include <memory>
#include <utility>
#include <cassert>

using std::shared_ptr;

template <typename T>
class persistent_set {
public:
    typedef T value_type;
private:
    struct base_node {
        base_node() {
            left = shared_ptr<base_node>();
            right = shared_ptr<base_node>();
        }

        base_node(base_node const& other) noexcept : left(other.left),
                                                     right(other.right)
        {}

        base_node(shared_ptr<base_node> l, shared_ptr<base_node> r): left(l),
                                                                     right(r)
        {}

        base_node &operator=(const base_node& other) {
            left = other.left;
            right = other.right;
            return *this;
        }

        virtual ~base_node() {
            left.reset();
            right.reset();
        }

        shared_ptr<base_node> left, right;
    };

    struct node : base_node {
        node() noexcept = delete;

        node(const node& other) : base_node(other),
                                  value(other.value)
        {}

        node(shared_ptr<base_node> l, shared_ptr<base_node> r, T val): base_node(l, r),
                                                                       value(val)
        {}

        explicit node(const T& val): base_node(nullptr, nullptr),
                                     value(val)
        {}

        node &operator=(const node& other) {
            base_node::left = other.left;
            base_node::right = other.right;
            value = other.value;
            return *this;
        }

        ~node() = default;

        T value;
    };

    shared_ptr<base_node> root;

    static T const &get_bn_val(const base_node* bn) {
        return static_cast<const node*>(bn)->value;
    }

    shared_ptr<base_node> insert(shared_ptr<base_node> x, T& val) {
        if (!x) {
            auto new_node = std::shared_ptr<base_node>(new node(val));
            return new_node;
        }

        shared_ptr<base_node> new_node(new node(get_bn_val(x.get())));
        shared_ptr<base_node> result;
        if (val == get_bn_val(x.get()))
            return x;
        if (val < get_bn_val(x.get())) {
            result = insert(x->left, val);
            new_node->left = result;
            new_node->right = x->right;
        }
        else {
            result = insert(x->right, val);
            new_node->left = x->left;
            new_node->right = result;
        }
        //result.second = new_node;
        return new_node;
    }

    shared_ptr<base_node> erase(base_node *it, T const &val) {
        if (get_bn_val(it) > val) {
            auto p = erase(it->left.get(), val);
            return shared_ptr<base_node>(new node(p, it->right, get_bn_val(it)));
        }
        if (get_bn_val(it) < val) {
            auto p = erase(it->right.get(), val);
            return shared_ptr<base_node>(new node(it->left, p, get_bn_val(it)));
        }
        if (it->left && it->right) {
            base_node* nxt = it->right.get();
            while (nxt->left)
                nxt = nxt->left.get();
            auto cp = get_bn_val(nxt);
            return shared_ptr<base_node>(new node(it->left, erase(it->right.get(), cp), cp));
        }
        if (it->left)
            return it->left;
        if (it->right)
            return it->right;
        return nullptr;
    }
public:
    struct iterator {
        iterator(base_node *h, base_node *c) : head(h),
                                               cur(c) {}

        iterator(iterator const &other) : head(other.head),
                                          cur(other.cur) {}

        T const &operator*() const {
            assert(cur != head);
            return get_bn_val(cur);
        }

        iterator &operator++() {
            cur = get_next();
            return *this;
        }

        iterator operator++(int) {
            iterator copy(*this);
            cur = get_next();
            return *this;
        }

        iterator &operator--() {
            cur = get_prev();
            return *this;
        }

        iterator operator--(int) {
            iterator copy(*this);
            cur = get_prev();
            return *this;
        }

        friend bool operator==(const iterator &a, const iterator &b) {
            return a.cur == b.cur;
        }

        friend bool operator!=(const iterator &a, const iterator &b) {
            return !(a == b);
        }

    private:
        base_node *head;
        base_node *cur;

        base_node *get_next() {
            base_node *p = head->left.get();
            base_node *successor = nullptr;
            while (p) {
                if (get_bn_val(p) > get_bn_val(cur)) {
                    successor = p;
                    p = p->left.get();
                } else
                    p = p->right.get();
            }
            if (!successor)
                return head;
            return successor;
        }

        base_node *get_prev() {
            if (cur == head)
                return get_max();
            base_node *p = head->left.get();
            base_node *successor = nullptr;
            while (p) {
                if (get_bn_val(p) < get_bn_val(cur)) {
                    successor = p;
                    p = p->right.get();
                } else
                    p = p->left.get();
            }
            return successor;
        }

        base_node *get_max() {
            auto p = head->left.get();
            while (p->right) p = p->right.get();
            return p;
        }
    };

    persistent_set() noexcept {
        root = shared_ptr<base_node>(new base_node());
    };

    persistent_set(persistent_set const &other) noexcept: persistent_set() {
        root->left = other.root->left;
    }

    persistent_set &operator=(persistent_set other) noexcept {
        swap(*this, other);
        return *this;
    }

    friend void swap(persistent_set& a, persistent_set& b) noexcept {
        std::swap(a.root, b.root);
    }

    ~persistent_set() {
        root.reset();
    }

    iterator find(value_type x) {
        if (!root->left)
            return iterator(root.get(), root.get());
        base_node* p = root->left.get();
        while (p) {
            if (x == get_bn_val(p))
                return iterator(root.get(), p);
            if (x < get_bn_val(p))
                p = p->left.get();
            else
                p = p->right.get();
        }
        return iterator(root.get(), root.get());
    }

    std::pair<iterator, bool> insert(T x) {
        auto it = find(x);
        if (it != end())
            return std::make_pair(it, false);
        auto result = insert(root->left, x);
        root->left = result;
        return std::make_pair(find(x), true);
    }

    void erase(iterator it) {
        root->left = erase(root->left.get(), *it);
    }

    iterator begin() const {
        auto p = root.get();
        while (p->left) p = p->left.get();
        return iterator(root.get(), p);
    }

    iterator end() const {
        return iterator(root.get(), root.get());
    }
};


#endif //PERSISTENT_SET_PERSISTENT_SET_H
