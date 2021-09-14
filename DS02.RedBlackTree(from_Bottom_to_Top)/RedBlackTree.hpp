//
// Created by SiriusNEO on 2021/9/13.
//

#ifndef DS02_REDBLACKTREE_FROM_BOTTOM_TO_TOP_REDBLACKTREE_HPP
#define DS02_REDBLACKTREE_FROM_BOTTOM_TO_TOP_REDBLACKTREE_HPP

#include <iostream>
#include <functional>
#include <cstddef>

namespace Sirius {

    /*
     * 红黑树, modified from Sirius's STLite map
     */

    template<class Key,
             class T,
             class Compare = std::less <Key>
             >
    class RedBlackTree {
    private:
        typedef std::pair<const Key, T> valueType;
        enum colorType {BLACK, RED};

        struct Node {
            valueType data;
            Node *father, *leftSon, *rightSon;
            colorType color;

            Node(const valueType &_data) : data(_data), color(RED), father(nullptr), leftSon(nullptr), rightSon(nullptr) {}

            void display() const {
                std::cout << "\n* --- Node " << this << " --- *\n";
                std::cout << "key: " << data.first << '\n';
                std::cout << "val: " << data.second << '\n';
                std::cout << "color: " << color << '\n';
                std::cout << "parent: " << father << '\n';
                std::cout << "leftSon: " << leftSon << '\n';
                std::cout << "rightSon: " << rightSon << '\n';
                if (leftSon != nullptr) leftSon->display();
                if (rightSon != nullptr) rightSon->display();
            }
        };

        typedef Node *NodeCur;

        bool getSide(NodeCur nowNode) const {
            return nowNode && nowNode->father && nowNode->father->rightSon == nowNode;
        }

        bool getColor(NodeCur nowNode) const { return nowNode && nowNode->color == RED; }

        NodeCur getBro(NodeCur nowNode) const {
            return (getSide(nowNode)) ? nowNode->father->leftSon : nowNode->father->rightSon;
        }

        void copy(NodeCur nowNode) { // 递归地拷贝一棵树, 用于实现map的拷贝构造
            if (nowNode == nullptr) return;
            insert(nowNode->data);
            if (nowNode->leftSon) copy(nowNode->leftSon);
            if (nowNode->rightSon) copy(nowNode->rightSon);
        }

        void link(NodeCur u, NodeCur v, bool side) {
            if (v) v->father = u;
            if (u) (side) ? u->rightSon = v : u->leftSon = v;
        }

        void clear(NodeCur nowNode) { // 从树中删除该节点
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

        void rotate(NodeCur nowNode) { // 子节点上旋
            if (nowNode == nullptr || nowNode->father == nullptr) return;
            if (root == nowNode->father) root = nowNode;
            NodeCur tmpFather = nowNode->father, tmpGrandpa = nowNode->father->father;
            bool side = getSide(nowNode->father);
            if (getSide(nowNode)) {
                link(tmpFather, nowNode->leftSon, 1);
                link(nowNode, tmpFather, 0);
                link(tmpGrandpa, nowNode, side);
            } else {
                link(tmpFather, nowNode->rightSon, 0);
                link(nowNode, tmpFather, 1);
                link(tmpGrandpa, nowNode, side);
            }
        }

        NodeCur search(const Key &_keyData) const {
            NodeCur targetNode = root;
            while (targetNode) {
                if (Compare()(_keyData, targetNode->data.first)) targetNode = targetNode->leftSon;
                else if (Compare()(targetNode->data.first, _keyData)) targetNode = targetNode->rightSon;
                else break;
            }
            return targetNode;
        }

        void insertFix(NodeCur nowNode) {
            if (nowNode == root) {
                root->color = BLACK;
                return;
            } else if (nowNode->father->color == BLACK) return;
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
            } else {
                rotate(nowNode->father);
                nowNode->father->color = BLACK;
                if (nowNode->father->leftSon) nowNode->father->leftSon->color = RED;
                if (nowNode->father->rightSon) nowNode->father->rightSon->color = RED;
            }
        }

        /*
         * 前继与后继, 用于实现迭代器
         */

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
            } else if (!getColor(broNode->leftSon) && !getColor(broNode->rightSon)) {
                if (getColor(broNode->father)) std::swap(broNode->father->color, broNode->color);
                else broNode->color = RED, delFix(broNode->father);
            } else if (getColor(broNode->leftSon)) {
                if (!getSide(broNode)) {
                    std::swap(broNode->color, broNode->father->color);
                    broNode->leftSon->color = BLACK;
                    rotate(broNode);
                } else {
                    std::swap(broNode->color, broNode->leftSon->color);
                    rotate(broNode->leftSon);
                    delFix(nowNode);
                }
            } else if (getColor(broNode->rightSon)) {
                if (getSide(broNode)) {
                    std::swap(broNode->color, broNode->father->color);
                    broNode->rightSon->color = BLACK;
                    rotate(broNode);
                } else {
                    std::swap(broNode->color, broNode->rightSon->color);
                    rotate(broNode->rightSon);
                    delFix(nowNode);
                }
            }
        }

        NodeCur root;
        size_t treeSize;

    public:
        RedBlackTree() : root(nullptr), treeSize(0) {}

        RedBlackTree(const RedBlackTree &obj) : root(nullptr), treeSize(0) { copy(obj.root); }

        ~RedBlackTree() { clear(root); }

        size_t size() const { return treeSize; }

        bool insert(const valueType &data) {
            NodeCur newNode = new Node(data), targetNode = root;
            ++treeSize;
            if (targetNode == nullptr) root = newNode;
            else {
                while (true) {
                    if (Compare()(data.first, targetNode->data.first)) {
                        if (targetNode->leftSon == nullptr) {
                            link(targetNode, newNode, 0);
                            break;
                        } else targetNode = targetNode->leftSon;
                    } else if (Compare()(targetNode->data.first, data.first)) {
                        if (targetNode->rightSon == nullptr) {
                            link(targetNode, newNode, 1);
                            break;
                        } else targetNode = targetNode->rightSon;
                    } else {
                        //key duplicate!
                        return false;
                    }
                }
            }
            insertFix(newNode);
            return true;
        }

        bool find(const Key &_keyData, T &val) const {
            NodeCur node = search(_keyData);
            if (node == nullptr) return false;
            val = node->data.second;
            return true;
        }

        void del(const Key &_keyData) {
            NodeCur targetNode = search(_keyData);
            while (targetNode->leftSon || targetNode->rightSon) {
                NodeCur replaceNode;
                if (targetNode->leftSon) replaceNode = pre(targetNode);
                else if (targetNode->rightSon) replaceNode = nxt(targetNode);
                NodeCur tmpFa = replaceNode->father, tmpLson = replaceNode->leftSon, tmpRson = replaceNode->rightSon;
                bool side = getSide(replaceNode);
                link(targetNode->father, replaceNode, getSide(targetNode));
                (targetNode->leftSon == replaceNode) ? link(replaceNode, targetNode, 0) : link(replaceNode,
                                                                                             targetNode->leftSon, 0);
                (targetNode->rightSon == replaceNode) ? link(replaceNode, targetNode, 1) : link(replaceNode,
                                                                                              targetNode->rightSon, 1);
                (tmpFa == targetNode) ? link(replaceNode, targetNode, side) : link(tmpFa, targetNode, side);
                link(targetNode, tmpLson, 0);
                link(targetNode, tmpRson, 1);
                std::swap(targetNode->color, replaceNode->color);
                if (root == targetNode) root = replaceNode;
            }
            delFix(targetNode);
            clear(targetNode);
        }

        void display() const {
            std::cout << "* --- RedBlackTree --- *\n";
            std::cout << "root: " << root << '\n';
            std::cout << "size: " << treeSize << '\n';
            if (root) {
                root->display();
            } else {
                std::cout << "<empty tree>\n";
            }
        }
    };
}

#endif //DS02_REDBLACKTREE_FROM_BOTTOM_TO_TOP_REDBLACKTREE_HPP
