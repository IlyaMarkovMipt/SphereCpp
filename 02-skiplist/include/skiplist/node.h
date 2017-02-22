#ifndef __NODE_H
#define __NODE_H

#include <cassert>

/**
 * Skiplist Node
 */
template<class Key, class Value>
class Node {
public:
    Node() {}

    virtual ~Node() {}

    /**
     * Return key assosiated with the given node
     */
    virtual const Key &key() const = 0;

    /**
     * Returns value assosiated with the given node
     */
    virtual const Value &value() const = 0;

    /**
     * Returns next node in the sequence
     */
    virtual Node* next() const = 0;

    virtual void next(Node<Key, Value>*) = 0;
};

/**
 * Skiplist data nodes that holds actual key/value pair
 */
template<class Key, class Value>
class DataNode : public Node<Key, Value> {
private:
    const Key *pKey;
    //Key key1;
    //Value value1;
    const Value *pValue;
    DataNode<Key, Value> *pNext;

public:
    DataNode(const Key *pKey_, const Value *pValue_)
            : pKey(pKey_), pValue(pValue_), pNext(nullptr) {
            if (pKey)
                pKey = new Key(*pKey);
            if (pValue)
               pValue = new Value(*pValue);
    }

    virtual ~DataNode() {
        delete pKey;
        delete pValue;
        pNext = nullptr;
    }

    /**
     * Return key assosiated with the given node
     */
    virtual const Key &key() const {
        assert(pKey != nullptr);

        return *pKey;
    }

    /**
     * Returns value assosiated with the given node
     */
    virtual const Value &value() const {
        assert(pValue != nullptr);
        return *pValue;
    };

    /**
     * Sets value assosiated with the given node
     */
    void value(const Value *value) {
        assert(pValue != nullptr);
        pValue = value;
    };

    /**
     * Returns next node in the sequence
     */
    virtual DataNode<Key, Value> *next() const {
        return pNext;
    };

    /**
     * Set next pointer
     */
    virtual void next(Node<Key, Value> *next) {
        pNext = (DataNode<Key, Value>*)next;
    };
};

/**
 * Skiplist index nodes that keep references onto data layer
 */
template<class Key, class Value>
class IndexNode : public Node<Key, Value> {
private:
    Node<Key, Value> *pDown;
    DataNode<Key, Value> *pRoot;
    IndexNode<Key, Value> *pNext;

public:
    IndexNode(Node<Key, Value> *down, DataNode<Key, Value> *root)
            : pDown(down), pRoot(root), pNext(nullptr) {
    }

    virtual ~IndexNode() {}

    /**
     * Return key assosiated with the given node
     */
    virtual const Key &key() const {
        assert(pRoot != nullptr);
        return pRoot->key();
    }

    /**
     * Returns value assosiated with the given node
     */
    virtual const Value &value() const {
        assert(pRoot != nullptr);
        return pRoot->value();
    };

    void setPDown(Node<Key, Value> *pDown) {
        IndexNode::pDown = pDown;
    }

    void setPRoot(DataNode<Key, Value> *pRoot) {
        IndexNode::pRoot = pRoot;
    }

    /**
     * Returns pointer down node of index node
     */
    Node<Key, Value> *down() const {
        return pDown;
    }

    DataNode<Key, Value>* root() const{
        return pRoot;
    };

    /**
     * Returns next node in the sequence
     */
    virtual IndexNode<Key, Value>* next() const {
        return pNext;
    };

    /**
     * Set next pointer
     */
    virtual void next(Node<Key, Value> *next) {
        pNext = (IndexNode<Key, Value>*) next;
    };
};

#endif // __NODE_H
