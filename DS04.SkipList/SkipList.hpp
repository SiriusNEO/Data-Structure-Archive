//
// Created by SiriusNEO on 2021/9/13.
//

#ifndef DS04_SKIPLIST_SKIPLIST_HPP
#define DS04_SKIPLIST_SKIPLIST_HPP

#include <iostream>
#include <random>
#include <functional>
#include <cstddef>

#define DEBUG(_x) //std::cout << _x << '\n';

namespace Sirius {

    /*
     * 跳表, 据说是 Redis 的底层数据结构
     * 0 为链表, MAX_LEVEL 为最高层 (包含)
     */

    template<class Key,
             class Val,
             int MAX_LEVEL = 16,
             class Compare = std::less<Key>
            >
    class SkipList {
    private:
        static constexpr double LEVEL_P = 0.5; // 浮点数不能作为 non-type 模板参数

        struct BaseNode {
            int level; // 节点的等级, 注意 level 向下为包含关系
            BaseNode *pre[MAX_LEVEL + 1], *nxt[MAX_LEVEL + 1]; // level i 的链表前后继, 双向链表设计

            virtual Key getKey() = 0;
            virtual Val getVal() = 0;
            virtual void display() = 0;

            BaseNode(int _level): level(_level) {
                for (int i = 0; i <= MAX_LEVEL; ++i) pre[i] = nxt[i] = nullptr;
            }
        };

        struct DataNode: public BaseNode {
            Key key;
            Val val;

            virtual Key getKey() {return key;}
            virtual Val getVal() {return val;}
            virtual void display() {
                std::cout << "Data(" << key << ", " << val << ")";
            }

            DataNode(const Key& _key, const Val& _val, int _level): key(_key), val(_val), BaseNode(_level) {}
        };

        struct HeadNode: public BaseNode {

            // 禁止调用, 对于 HeadNode 此两类方法无意义
            virtual Key getKey() {throw "try to get key from HeadNode";}
            virtual Val getVal() {throw "try to get val from HeadNode";}

            explicit HeadNode(int _level): BaseNode(_level) {}

            virtual void display() {
                std::cout << "Head";
            }
        };

        typedef BaseNode* NodeCur;

        size_t siz;
        NodeCur head[MAX_LEVEL + 1];
        int nowMaxLevel;

        /*
         * 随机层数, 从 0 层开始不断随机, 有 p 的概率往上
         */
        static int randomLevel() {
            int threshold = 100 * LEVEL_P; // 100 起放大作用, 精确到小数点后 2 位
            int returnLevel = 0;
            while (returnLevel < MAX_LEVEL) {
                int hitPos = rand() % 100 + 1;
                if (hitPos > threshold) break;
                ++returnLevel;
            }
            return returnLevel;
        }

    public:
        SkipList(): siz(0), nowMaxLevel(0) {
            srand(time(NULL));
            NodeCur newHead = new HeadNode(MAX_LEVEL);
            for (int i = 0; i <= MAX_LEVEL; ++i) head[i] = newHead;
        }

        ~SkipList() { // 注意, 链表析构不能写成递归形式, 数据量10w多就会爆栈
            NodeCur nowNode = head[0], nxtNode = head[0]->nxt[0];
            while (nowNode) {
                delete nowNode;
                nowNode = nxtNode;
                if (nxtNode) nxtNode = nxtNode->nxt[0];
            }
        }

        bool insert(const Key& key, const Val& val) {
            int newNodeLevel = randomLevel();
            NodeCur newNode = new DataNode(key, val, newNodeLevel);
            NodeCur preNode = head[newNode->level];

            for (int i = MAX_LEVEL; i >= 0; --i) {
                //DEBUG("what: " << preNode->nxt[i])
                while (preNode->nxt[i] != nullptr && Compare()(preNode->nxt[i]->getKey(), key)) {
                    DEBUG("tracing " << "key: " << key << " nxt-key: " << preNode->nxt[i]->getKey())
                    preNode = preNode->nxt[i];
                }
                if (i <= newNodeLevel) {
                    if (preNode->nxt[i]) {
                        if (!Compare()(key, preNode->nxt[i]->getKey())) {
                            DEBUG("key duplicate!")
                            delete newNode;
                            return false;
                        }
                        preNode->nxt[i]->pre[i] = newNode;
                        newNode->nxt[i] = preNode->nxt[i];
                    }
                    preNode->nxt[i] = newNode;
                    newNode->pre[i] = preNode;
                }
            }
            siz++;
            if (newNodeLevel > nowMaxLevel) nowMaxLevel = newNodeLevel;

            DEBUG("[insert successfully] key: " << key << " val: " << val << " level: " << newNodeLevel)

            return true;
        }

        bool find(const Key& key, Val& val) const {
            NodeCur node = head[nowMaxLevel];
            for (int i = nowMaxLevel; i >= 0; --i) {
                while (node->nxt[i] != nullptr && Compare()(node->nxt[i]->getKey(), key))
                    node = node->nxt[i];
                if (node->nxt[i] && !Compare()(key, node->nxt[i]->getKey())) {
                    DEBUG("found!")
                    val = node->nxt[i]->getVal();
                    return true;
                }
            }
            return false;
        }

        bool del(const Key& key) {
            NodeCur node = head[nowMaxLevel];
            for (int i = nowMaxLevel; i >= 0; --i) {
                while (node->nxt[i] != nullptr && Compare()(node->nxt[i]->getKey(), key))
                    node = node->nxt[i];
                if (node->nxt[i] && !Compare()(key, node->nxt[i]->getKey())) {
                    NodeCur delNode = node->nxt[i];
                    for (int j = i; j >= 0; --j) {
                        if (delNode->pre[j] == head[j] && delNode->nxt[j] == nullptr && j > 0) {
                            --nowMaxLevel; // 第 j 层置空, 0 层置空时仍显示最大层数为 0
                        }
                        if (delNode->nxt[j]) {
                            delNode->nxt[j]->pre[j] = delNode->pre[j];
                        }
                        delNode->pre[j]->nxt[j] = delNode->nxt[j];
                    }
                    delete delNode;
                    --siz;
                    return true;
                }
            }
            return false;
        }

        void display() const {
            std::cout << "* --- SkipList --- *\n";
            std::cout << "size: " << siz << '\n';
            std::cout << "nowMaxLevel: " << nowMaxLevel << '\n';

            if (siz) {
                for (int i = nowMaxLevel; i >= 0; --i) {
                    std::cout << "* Level " << i << ": ";
                    NodeCur node = head[i];
                    while (node != nullptr) {
                        node->display();
                        if (node->nxt[i]) std::cout << "->";
                        node = node->nxt[i];
                    }
                    std::cout << '\n';
                }
            } else {
                std::cout << "<empty>\n";
            }
        }
    };

}

#endif //DS04_SKIPLIST_SKIPLIST_HPP
