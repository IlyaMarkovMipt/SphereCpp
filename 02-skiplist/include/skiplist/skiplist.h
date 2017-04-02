#ifndef __SKIPLIST_H
#define __SKIPLIST_H

#include <functional>
#include "node.h"
#include "iterator.h"


#include <cstdlib>
#include <ctime>

/**
 * Skiplist interface
 */
template<class Key, class Value, size_t MAXHEIGHT, class Compare = std::less<Key>>
class SkipList {
private:
    DataNode<Key, Value> *pHead;
    DataNode<Key, Value> *pTail;

    IndexNode<Key, Value> *pTailIdx;
    IndexNode<Key, Value> *aHeadIdx[MAXHEIGHT];
    Compare cmp = Compare();

public:
    /**
     * Creates new empty skiplist
     */
    SkipList() {
        pHead = new DataNode<Key, Value>(nullptr, nullptr);
        pTail = new DataNode<Key, Value>(nullptr, nullptr);
        pHead->next(pTail);
        Node<Key, Value> *prev = pHead;
        pTailIdx = new IndexNode<Key, Value>(pTail, pTail);
        for (int i = 0; i < MAXHEIGHT; i++) {
            aHeadIdx[i] = new IndexNode<Key, Value>(prev, pHead);
            aHeadIdx[i]->next(pTailIdx);
            prev = aHeadIdx[i];
        }
        srand((unsigned) time(0));
    }

    /**
     * Disable copy constructor
     */
    SkipList(const SkipList &that) = delete;

    /**
     * Destructor
     */
    virtual ~SkipList() {
        for (int i = 0; i < MAXHEIGHT; i++) {
            IndexNode<Key, Value> *cur = aHeadIdx[i];
            IndexNode<Key, Value> *tmp;
            while (cur != pTailIdx){
                tmp = cur->next();
                delete cur;
                cur = tmp;
            }
        }
        DataNode<Key, Value>* cur = pHead;
        DataNode<Key, Value>* tmp;
        while (cur != pTail){
            tmp = cur->next();
            delete cur;
            cur = tmp;
        }
        delete pTailIdx;
        delete pTail;
    }

    /**
     * Assign new value for the key. If a such key already has
     * assosiation then old value returns, otherwise nullptr
     *
     * @param key key to be assigned with value
     * @param value to be added
     * @return old value for the given key or nullptr
     */
    virtual Value *Put(const Key &key, const Value &value) {
        Node<Key, Value> *prevs[MAXHEIGHT];
        find_(key, prevs);
        DataNode<Key, Value> *data_node = (DataNode<Key, Value> *) prevs[0];
        DataNode<Key, Value> *node_next =  data_node->next();

        if (node_next != pTail) {
            Key next_key = node_next->key();

            if (!(cmp(key, next_key) || cmp(next_key, key))) {
                Value *val = new Value(node_next->value());
                //keys are equal
                // need to change and add more stairs if lucky
                node_next->value(&value);
                // add more stairs
                Node<Key, Value> *down = node_next;
		int flip = rand();
                for (int i = 1; i < MAXHEIGHT + 1; i++) {
                    IndexNode<Key, Value> *index_node = (IndexNode<Key, Value> *) prevs[i];
                    if ((index_node->next())->root() != node_next) {
                        // we have found index_node which not above our node_next
                        if (flip & 1 == 1) {
                            IndexNode<Key, Value> *new_ = new IndexNode<Key, Value>(down, node_next);
                            new_->next(index_node->next());
                            index_node->next(new_);
                            down = new_;
			    flip >>= 1;
                        } else {
                            break;
                        }
                    }
                }
                return val;
            }
        }
        put_(key, value, prevs);
        return nullptr;
    };

    /**
     * Put value only if there is no assosiation with key in
     * the list and returns nullptr
     *
     * If there is an established assosiation with the key already
     * method doesn't nothing and returns existing value
     *
     * @param key key to be assigned with value
     * @param value to be added
     * @return existing value for the given key or nullptr
     */
    virtual Value *PutIfAbsent(const Key &key, const Value &value) {
        Node<Key, Value> *prevs[MAXHEIGHT];
        find_(key, prevs);
        DataNode<Key, Value> *data_node = (DataNode<Key, Value> *) prevs[0];
        DataNode<Key, Value> *node_next =  data_node->next();
        if (node_next != pTail) {
            Key next_key = node_next->key();
            Value *val = new Value(node_next->value());
            if (!(cmp(key, next_key) || cmp(next_key, key))) {
                return val;
            }
        }
        put_(key, value, prevs);
        return nullptr;
    };

    /**
     * Returns value assigned for the given key or nullptr
     * if there is no established assosiation with the given key
     *
     * @param key to find
     * @return value assosiated with given key or nullptr
     */
    virtual Value *Get(const Key &key) const {

        Node<Key, Value> *prevs[MAXHEIGHT];
        find_(key, prevs);
        DataNode<Key, Value> *node_next =  ((DataNode<Key, Value> *)prevs[0])->next();

        if (node_next != pTail) {
            Key next_key = node_next->key();
            if (!(cmp(key, next_key) || cmp(next_key, key))) {
                return new Value(node_next->value());
            }
        }
        return nullptr;
    };

    /**
     * Remove given key from the skpiplist and returns value
     * it has or nullptr in case if key wasn't assosiated with
     * any value
     *
     * @param key to be added
     * @return value for the removed key or nullptr
     */
    virtual Value *Delete(const Key &key) {
        Node<Key, Value> *prevs[MAXHEIGHT];
        find_(key, prevs);
        DataNode<Key, Value> *node_next =  ((DataNode<Key, Value> *)prevs[0])->next();
        
        if (node_next != pTail) {
            for (int i = 1; i < MAXHEIGHT + 1; i++) {
				IndexNode<Key, Value> *next = (IndexNode<Key, Value> *) prevs[i]->next();//candidate to be deleted
            	if (!(next->root() == pHead
										|| next == pTailIdx 
										|| cmp(next->key(), key)
										|| cmp(key, next->key()))) {
                    prevs[i]->next(next->next());
                    delete next;
				} else {
					break;
				} 
            }
			if (!(cmp(key, node_next->key()) || cmp(node_next->key(), key))) {
				prevs[0]->next(node_next->next());
				Value *val = new Value(node_next->value());
				delete node_next;
				return val;
			}
        }
      	return nullptr;
    };

    /**
     * Same as Get
     */
    virtual const Value *operator[](const Key &key) {
        return Get(key);
    };

    /**
     * Return iterator onto very first key in the skiplist
     */
    virtual Iterator<Key, Value> cbegin() const {
        return Iterator<Key, Value>(pHead->next());
    };

    /**
     * Returns iterator to the first key that is greater or equals to
     * the given key
     */
    virtual Iterator<Key, Value> cfind(const Key &min) const {
        Node<Key, Value> *prevs[MAXHEIGHT];
        find_(min, prevs);
        return Iterator<Key, Value>(prevs[0]->next());
    };

    /**
     * Returns iterator on the skiplist tail
     */
    virtual Iterator<Key, Value> cend() const {
        return Iterator<Key, Value>(pTail);
    };

    void dump(){
    	print(aHeadIdx[MAXHEIGHT - 1]);
    }
private:

    void print(IndexNode<Key, Value>* node) const{
        Node<Key, Value>* cur = node;
        printf("pHead:%p \n", pHead);
        printf("pTail:%p \n", pTail);
        printf("pTailIdx: %p \n", pTailIdx);
        printf("Index layer \n");
        while (cur != node->root()){
            IndexNode<Key, Value>* cur_run = (IndexNode<Key, Value>*)cur;
            while (cur_run != pTailIdx) {
                printf("%p(%i) ->", cur_run, (cur_run->root() == pHead)?0:cur_run->key());
                cur_run = cur_run->next();
            }
            printf("%p \n", cur_run);
            cur = ((IndexNode<Key,Value>*)cur)->down();
        }
        printf("Data layer \n");
        while (cur != pTail){
            printf("%p:%i ->", cur, (cur == pHead)?0:cur->key());
            cur = cur->next();
        }
        printf("%p\n", cur);
    }
    /**
     * Returns the Data_node, previous to one the given key would be inserted to, and previous index_nodes
     */
    void find_(const Key &key, Node<Key, Value>** prevs) const{
        Node<Key, Value> *cur = aHeadIdx[MAXHEIGHT - 1]; // start from top
        size_t height = MAXHEIGHT;
        while (height > 0) {
            Node<Key, Value> *next = cur->next();
            if (next == pTailIdx) {
                prevs[height--] = cur;
                cur = ((IndexNode<Key, Value> *) cur)->down();
            } else {
		    //It's ok here to call cmp, because cmp == less, of sth like bool return function
                if (cmp(next->key(), key)) {
                    // if key > next_key we go tj next index node
                    cur = next;
                } else {
					// I need to have all previous index nodes without skipping 
					// down in case of equality
                    // key <= next->key() we go down
                    prevs[height--] = cur;
                    cur = ((IndexNode<Key, Value> *) cur)->down();
                }
            }
        }
		// it's better to save this logic with final move on the last layer
		// instead of thinking whether call down() or not inmain cycle 
        while (cur->next() != pTail && cmp(cur->next()->key(), key)) {
            cur = cur->next(); // here we find the closest node in the lowest level
        }
        prevs[0] = cur;
    }

    void put_(const Key& key, const Value& value, Node<Key, Value>** prevs) const{
        DataNode<Key, Value>* data_node = (DataNode<Key, Value>*) prevs[0];
        DataNode<Key, Value>* new_node = new DataNode<Key, Value>(&key, &value);
        new_node->next(data_node->next());
        data_node->next(new_node);
        Node<Key, Value>* down = new_node;
	int flip = rand();
        for (int i = 1; i < MAXHEIGHT + 1; i++){
            if (flip & 1 == 1){
                IndexNode<Key, Value>* prev_index = (IndexNode<Key, Value>*)prevs[i];
                IndexNode<Key, Value>* new_ = new IndexNode<Key, Value>(down , new_node);
                new_->next(prev_index->next());
                prev_index->next(new_);
                down = new_;
		flip >>= 1;
            } else {
                break;
            }
        }
    }


};

#endif // __SKIPLIST_H
