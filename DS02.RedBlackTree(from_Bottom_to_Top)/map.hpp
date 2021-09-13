/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

#define BOMB std::cout << "bomb\n";

namespace sjtu {

template<
        class Key,
        class T,
        class Compare
> class map;

template<
        class Key,
        class T,
        class Compare = std::less<Key>
> class RedBlackTree {
public:
    typedef pair<const Key, T> value_type;
    enum colorType {BLACK, RED};
    struct Node {
        value_type data;
        Node *father, *leftSon, *rightSon;
        colorType color;
        Node(const value_type& _data):data(_data),color(RED),
                               father(nullptr),leftSon(nullptr),rightSon(nullptr){}
    };
    typedef Node* NodeCur;
    RedBlackTree():root(nullptr),treeSize(0),firstNode(nullptr),lastNode(nullptr){}
    RedBlackTree(const RedBlackTree& obj):root(nullptr),treeSize(0) {copy(obj.root);}
    ~RedBlackTree(){clear(root);}

    size_t size() const {return treeSize;}
    bool getSide(NodeCur nowNode) const {return nowNode && nowNode->father && nowNode->father->rightSon == nowNode;}
    bool getColor(NodeCur nowNode) const {return nowNode && nowNode->color == RED;}
    NodeCur getBro(NodeCur nowNode) const {return (getSide(nowNode)) ? nowNode->father->leftSon : nowNode->father->rightSon;}

    void copy(NodeCur nowNode) {
        if (nowNode == nullptr) return;
        insert(nowNode->data);
        if (nowNode->leftSon) copy(nowNode->leftSon);
        if (nowNode->rightSon) copy(nowNode->rightSon);
    }

    void link(NodeCur u, NodeCur v, bool side) {
        if (v) v->father = u;
        if (u) (side) ? u->rightSon = v : u->leftSon = v;
    }

    void clear(NodeCur nowNode) {
        if (nowNode == nullptr) return;
        if (nowNode == root) root = nullptr;
        if (nowNode->father) {
            if (getSide(nowNode)) nowNode->father->rightSon = nullptr;
            else nowNode->father->leftSon = nullptr;
        }
        --treeSize;
        if (nowNode->leftSon) clear(nowNode->leftSon);
        if (nowNode->rightSon) clear(nowNode->rightSon);
        delete nowNode;
    }

    void rotate(NodeCur nowNode) {
        if (nowNode == nullptr || nowNode->father == nullptr) return;
        if (root == nowNode->father) root = nowNode;
        NodeCur tmpFather = nowNode->father, tmpGrandpa = nowNode->father->father;
        bool side = getSide(nowNode->father);
        if (getSide(nowNode)) {
            link(tmpFather, nowNode->leftSon, 1);
            link(nowNode, tmpFather, 0);
            link(tmpGrandpa, nowNode, side);
        }
        else {
            link(tmpFather, nowNode->rightSon, 0);
            link(nowNode, tmpFather, 1);
            link(tmpGrandpa, nowNode, side);
        }
    }

    void insertFix(NodeCur nowNode) {
        if (nowNode == root) {
            root->color = BLACK;
            return;
        }
        else if (nowNode->father->color == BLACK) return;
        NodeCur uncleNode = getBro(nowNode->father);
        if (getColor(uncleNode)) {
            nowNode->father->color = BLACK;
            uncleNode->color = BLACK;
            nowNode->father->father->color = RED;
            insertFix(nowNode->father->father);
            return;
        }
        if (getSide(nowNode) != getSide(nowNode->father)) {
            rotate(nowNode), rotate(nowNode);
            nowNode->color = BLACK;
            if (nowNode->leftSon) nowNode->leftSon->color = RED;
            if (nowNode->rightSon) nowNode->rightSon->color = RED;
        }
        else {
            rotate(nowNode->father);
            nowNode->father->color = BLACK;
            if (nowNode->father->leftSon) nowNode->father->leftSon->color = RED;
            if (nowNode->father->rightSon) nowNode->father->rightSon->color = RED;
        }
    }

    NodeCur insert(const value_type& data) {
        NodeCur newNode = new Node(data), targetNode = root;
        ++treeSize;
        if (targetNode == nullptr) root = firstNode = lastNode = newNode;
        else {
            while (true) {
                if (Compare()(data.first, targetNode->data.first)) {
                    if (targetNode->leftSon == nullptr) {
                        link(targetNode, newNode, 0);
                        if (firstNode == targetNode) firstNode = newNode;
                        break;
                    } else targetNode = targetNode->leftSon;
                } else {
                    if (targetNode->rightSon == nullptr) {
                        link(targetNode, newNode, 1);
                        if (lastNode == targetNode) lastNode = newNode;
                        break;
                    } else targetNode = targetNode->rightSon;
                }
            }
        }
        insertFix(newNode);
        return newNode;
    }

    NodeCur search(const Key& _keyData) const {
        NodeCur targetNode = root;
        while (targetNode) {
            if (Compare()(_keyData, targetNode->data.first)) targetNode = targetNode->leftSon;
            else if (Compare()(targetNode->data.first, _keyData)) targetNode = targetNode->rightSon;
            else break;
        }
        return targetNode;
    }

    NodeCur pre(NodeCur nowNode) const {
        if (nowNode == nullptr) return nullptr;
        if (nowNode->leftSon) {
            nowNode = nowNode->leftSon;
            while (nowNode->rightSon) nowNode = nowNode->rightSon;
            return nowNode;
        }
        while (nowNode->father && !getSide(nowNode)) nowNode = nowNode->father;
        return nowNode->father;
    }

    NodeCur nxt(NodeCur nowNode) const {
        if (nowNode == nullptr) return nullptr;
        if (nowNode->rightSon) {
            nowNode = nowNode->rightSon;
            while (nowNode->leftSon) nowNode = nowNode->leftSon;
            return nowNode;
        }
        while (nowNode->father && getSide(nowNode)) nowNode = nowNode->father;
        return nowNode->father;
    }

    NodeCur first() const {
        NodeCur ret = root;
        while (ret && ret->leftSon) ret = ret->leftSon;
        return ret;
    }

    NodeCur last() const {
        NodeCur ret = root;
        while (ret && ret->rightSon) ret = ret->rightSon;
        return ret;
    }

    void delFix(NodeCur nowNode) { //to make the bro's subtree black height-1
        if (getColor(nowNode) || nowNode == root) return;
        NodeCur broNode = getBro(nowNode);
        if (getColor(broNode)) {
            std::swap(broNode->color, broNode->father->color);
            rotate(broNode);
            delFix(nowNode);
        }
        else if (!getColor(broNode->leftSon) && !getColor(broNode->rightSon)) {
            if (getColor(broNode->father)) std::swap(broNode->father->color, broNode->color);
            else broNode->color = RED, delFix(broNode->father);
        }
        else if (getColor(broNode->leftSon)) {
            if (!getSide(broNode)) {
                std::swap(broNode->color, broNode->father->color);
                broNode->leftSon->color = BLACK;
                rotate(broNode);
            }
            else {
                std::swap(broNode->color, broNode->leftSon->color);
                rotate(broNode->leftSon);
                delFix(nowNode);
            }
        }
        else if (getColor(broNode->rightSon)) {
            if (getSide(broNode)) {
                std::swap(broNode->color, broNode->father->color);
                broNode->rightSon->color = BLACK;
                rotate(broNode);
            }
            else {
                std::swap(broNode->color, broNode->rightSon->color);
                rotate(broNode->rightSon);
                delFix(nowNode);
            }
        }
    }

    void del(const Key& _keyData) {
        NodeCur targetNode = search(_keyData);
        while (targetNode->leftSon || targetNode->rightSon) {
            NodeCur neiborNode;
            if (targetNode->leftSon) neiborNode = pre(targetNode);
            else if (targetNode->rightSon) neiborNode = nxt(targetNode);
            NodeCur tmpFa = neiborNode->father, tmpLson = neiborNode->leftSon, tmpRson = neiborNode->rightSon;
            bool side = getSide(neiborNode);
            link(targetNode->father, neiborNode, getSide(targetNode));
            (targetNode->leftSon == neiborNode) ? link(neiborNode, targetNode, 0) : link(neiborNode, targetNode->leftSon, 0);
            (targetNode->rightSon == neiborNode) ? link(neiborNode, targetNode, 1) : link(neiborNode, targetNode->rightSon, 1);
            (tmpFa == targetNode) ? link(neiborNode, targetNode, side) : link(tmpFa, targetNode, side);
            link(targetNode, tmpLson, 0);
            link(targetNode, tmpRson, 1);
            std::swap(targetNode->color, neiborNode->color);
            if (root == targetNode) root = neiborNode;
        }
        delFix(targetNode);
        clear(targetNode);
        if (targetNode == firstNode) firstNode = first();
        else if (targetNode == lastNode) lastNode = last();
    }
private:
    NodeCur root, firstNode, lastNode;
    size_t treeSize;
    friend map<Key, T, Compare>;
};

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {
private:
    RedBlackTree<Key, T, Compare> redBlackTree;
    friend RedBlackTree<Key, T, Compare>;
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::map as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		 RedBlackTree<Key, T, Compare>* dataStructure;
		 typename RedBlackTree<Key, T, Compare>::NodeCur node;
         friend map<Key, T, Compare>;
	public:
		iterator() {
			dataStructure = nullptr, node = nullptr;
		}
        iterator(RedBlackTree<Key, T, Compare>* _dataStructure, typename RedBlackTree<Key, T, Compare>::NodeCur _node):
        dataStructure(_dataStructure), node(_node){
		}
		iterator(const iterator &other):dataStructure(other.dataStructure), node(other.node) {}
        iterator operator++(int) {
            if (node == nullptr) throw invalid_iterator();
            iterator ret = *this;
            node = dataStructure->nxt(node);
            return ret;
        }
        /**
         * TODO ++iter
         */
        iterator & operator++() {
            if (node == nullptr) throw invalid_iterator();
            node = dataStructure->nxt(node);
            return *this;
        }
        /**
         * TODO iter--
         */
        iterator operator--(int) {
            typename RedBlackTree<Key, T, Compare>::NodeCur preNode;
            preNode = (node == nullptr) ? dataStructure->lastNode : dataStructure->pre(node);
            if (preNode == nullptr) throw invalid_iterator();
            iterator ret = *this;
            node = preNode;
            return ret;
        }
        /**
         * TODO --iter
         */
        iterator & operator--() {
            typename RedBlackTree<Key, T, Compare>::NodeCur preNode;
            preNode = (node == nullptr) ? dataStructure->lastNode : dataStructure->pre(node);
            if (preNode == nullptr) throw invalid_iterator();
            node = preNode;
            return *this;
        }
		/**
		 * an operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {return node->data;}
		iterator & operator = (const iterator &rhs) {
		    if (this == &rhs) return *this;
		    dataStructure = rhs.dataStructure;
		    node = rhs.node;
		    return *this;
		}
        iterator & operator = (const const_iterator &rhs) {
            if (this == &rhs) return *this;
            dataStructure = rhs.dataStructure;
            node = rhs.node;
            return *this;
        }
		bool operator==(const iterator &rhs) const {return node==rhs.node&&dataStructure==rhs.dataStructure;}
		bool operator==(const const_iterator &rhs) const {return node==rhs.node&&dataStructure==rhs.dataStructure;}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {return node!=rhs.node||dataStructure!=rhs.dataStructure;}
		bool operator!=(const const_iterator &rhs) const {return node!=rhs.node||dataStructure!=rhs.dataStructure;}

		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {return &node->data;}
	};
	class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
    private:
        // data members.
        const RedBlackTree<Key, T, Compare>* dataStructure;
        typename RedBlackTree<Key, T, Compare>::NodeCur node;
        friend map<Key, T, Compare>;
    public:
        const_iterator():dataStructure(nullptr), node(nullptr) {}
        const_iterator(const RedBlackTree<Key, T, Compare>* _dataStructure, typename RedBlackTree<Key, T, Compare>::NodeCur _node):
        dataStructure(_dataStructure), node(_node){}
        const_iterator(const const_iterator &other):dataStructure(other.dataStructure), node(other.node) {}
        explicit const_iterator(const iterator &other):dataStructure(other.dataStructure), node(other.node) {}
        // And other methods in iterator.
        // And other methods in iterator.
        // And other methods in iterator.
        const_iterator operator++(int) {
            if (node == nullptr) throw invalid_iterator();
            const_iterator ret = *this;
            node = dataStructure->nxt(node);
            return ret;
        }
        /**
         * TODO ++iter
         */
        const_iterator & operator++() {
            if (node == nullptr) throw invalid_iterator();
            node = dataStructure->nxt(node);
            return *this;
        }
        /**
         * TODO iter--
         */
        const_iterator operator--(int) {
            typename RedBlackTree<Key, T, Compare>::NodeCur preNode;
            preNode = (node == nullptr) ? dataStructure->lastNode : dataStructure->pre(node);
            if (preNode == nullptr) throw invalid_iterator();
            const_iterator ret = *this;
            node = preNode;
            return ret;
        }
        /**
         * TODO --iter
         */
        const_iterator & operator--() {
            typename RedBlackTree<Key, T, Compare>::NodeCur preNode;
            preNode = (node == nullptr) ? dataStructure->lastNode : dataStructure->pre(node);
            if (preNode == nullptr) throw invalid_iterator();
            node = preNode;
            return *this;
        }
        /**
         * an operator to check whether two iterators are same (pointing to the same memory).
         */
        const value_type & operator*() const {return node->data;}
        const_iterator & operator = (const iterator &rhs) {
            dataStructure = rhs.dataStructure;
            node = rhs.node;
            return *this;
        }
        const_iterator & operator = (const const_iterator &rhs) {
            if (this == &rhs) return *this;
            dataStructure = rhs.dataStructure;
            node = rhs.node;
            return *this;
        }
        bool operator==(const iterator &rhs) const {return node==rhs.node&&dataStructure==rhs.dataStructure;}
        bool operator==(const const_iterator &rhs) const {return node==rhs.node&&dataStructure==rhs.dataStructure;}
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {return node!=rhs.node||dataStructure!=rhs.dataStructure;}
        bool operator!=(const const_iterator &rhs) const {return node!=rhs.node||dataStructure!=rhs.dataStructure;}
        const value_type* operator->() const noexcept {return &node->data;}
	};
	/**
	 * TODO two constructors
	 */
	map():redBlackTree() {}
	map(const map &other):redBlackTree(other.redBlackTree) {}
	/**
	 * TODO assignment operator
	 */
	map & operator=(const map &other) {
	    if (this == &other) return *this;
	    clear();
	    redBlackTree.copy(other.redBlackTree.root);
	    return *this;
	}
	/**
	 * TODO Destructors
	 */
	~map() {}
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
	    typename RedBlackTree<Key, T, Compare>::NodeCur node = redBlackTree.search(key);
        if (node == nullptr) throw index_out_of_bound();
        return node->data.second;
	}
	const T & at(const Key &key) const {
        typename RedBlackTree<Key, T, Compare>::NodeCur node = redBlackTree.search(key);
        if (node == nullptr) throw index_out_of_bound();
        return node->data.second;
	}
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
        typename RedBlackTree<Key, T, Compare>::NodeCur node = redBlackTree.search(key);
        if (node == nullptr) {
            T defaultData;
            value_type value(key, defaultData);
            return redBlackTree.insert(value)->data.second;
        }
        return node->data.second;
	}
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
        typename RedBlackTree<Key, T, Compare>::NodeCur node = redBlackTree.search(key);
        if (node == nullptr) throw index_out_of_bound();
        return node->data.second;
	}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {return iterator(&redBlackTree, redBlackTree.firstNode);}
	const_iterator cbegin() const {return const_iterator(&redBlackTree, redBlackTree.firstNode);}
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {return iterator(&redBlackTree, nullptr);}
	const_iterator cend() const {return const_iterator(&redBlackTree, nullptr);}
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {return redBlackTree.root == nullptr;}
	/**
	 * returns the number of elements.
	 */
	size_t size() const {return redBlackTree.size();}
	/**
	 * clears the contents
	 */
	void clear() {
	    redBlackTree.clear(redBlackTree.root);
	    redBlackTree.firstNode = redBlackTree.lastNode = nullptr;
	}
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
	    auto findNode = redBlackTree.search(value.first);
	    if (findNode == nullptr) return pair<iterator, bool>(iterator(&redBlackTree, redBlackTree.insert(value)), true);
	    return pair<iterator, bool>(iterator(&redBlackTree, findNode), false);
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
	    if (pos.node == nullptr || pos.dataStructure != &redBlackTree) throw invalid_iterator();
	    redBlackTree.del(pos.node->data.first);
	}
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */
	size_t count(const Key &key) const {return redBlackTree.search(key) != nullptr;}
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {return iterator(&redBlackTree, redBlackTree.search(key));}
	const_iterator find(const Key &key) const {return const_iterator(&redBlackTree, redBlackTree.search(key));}
};

}

#endif
