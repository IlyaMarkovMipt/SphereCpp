#ifndef __ITERATOR_H
#define __ITERATOR_H

#include <cassert>
#include "node.h"

/**
 * Skiplist const iterator
 */
template<class Key, class Value>
class Iterator {
private:
    Node<Key, Value> *pCurrent;

public:
    Iterator(Node<Key, Value> *p) : pCurrent(p) {}

    virtual ~Iterator() {}

    virtual const Key &key() const {
        assert(pCurrent != nullptr);
        return pCurrent->key();
    };

    virtual const Value &value() const {
        assert(pCurrent != nullptr);
        return pCurrent->value();
    };

    virtual const Value &operator*() {
        assert(pCurrent != nullptr);
        return pCurrent->value();
    };

    virtual const Value &operator->() {
        assert(pCurrent != nullptr);
        return pCurrent->value();
    };

    virtual bool operator==(const Iterator &it) const {
        return pCurrent == it.pCurrent;
    };

    virtual bool operator!=(const Iterator &it) const {
        return pCurrent != it.pCurrent;
    };

    virtual Iterator &operator=(const Iterator &it) {
        pCurrent = it.pCurrent;
        return *this;
    };

    virtual Iterator &operator++() {
        Iterator prev = *this;
        pCurrent = pCurrent->next();
        return prev;
    };

    virtual Iterator &operator++(int) {
        pCurrent = pCurrent->next();
        return *this;
    };
};

#endif // __ITERATOR_H
