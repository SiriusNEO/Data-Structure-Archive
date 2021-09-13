//
// Created by SiriusNEO on 2021/9/13.
//

#ifndef DS03_AVLTREE_AVLTREE_HPP
#define DS03_AVLTREE_AVLTREE_HPP

#include <iostream>
#include <functional>
#include <cstddef>

namespace Sirius {

#define DEBUG(_x) std::cout << _x << '\n';

    /*
     * AVL树
     */
    template<class Key, class Val, class Compare = std::less<Key>> // 小于, strong compare
    class AVLTree {

        enum SideType {NULL_SIDE, LEFT, RIGHT};

    private:
        struct AVLNode {
            int height; // 平衡因子: leftSon->height - rightSon->height
            Key key;
            Val val;
            AVLNode *parent, *leftSon, *rightSon;

            AVLNode(const Key& _key, const Val& _val): height(1), key(_key), val(_val), parent(nullptr), leftSon(nullptr), rightSon(nullptr) {}

            ~AVLNode() {
                if (leftSon != nullptr) delete leftSon;
                if (rightSon != nullptr) delete rightSon;
            }

            void display() const {
                std::cout << "\n* --- AVLNode " << this << " --- *\n";
                std::cout << "key: " << key << '\n';
                std::cout << "val: " << val << '\n';
                std::cout << "height: " << height << '\n';
                std::cout << "parent: " << parent << '\n';
                std::cout << "leftSon: " << leftSon << '\n';
                std::cout << "rightSon: " << rightSon << '\n';
                if (leftSon != nullptr) this->leftSon->display();
                if (rightSon != nullptr) this->rightSon->display();
            }
        };

        size_t siz;
        AVLNode *root;

        /*
         * internal
         */

        SideType getSide(AVLNode *node) const { // 获得 node 相对于其 parent 的边
            if (node->parent == nullptr) return NULL_SIDE;
            if (node->parent->leftSon == node) return LEFT;
            return RIGHT;
        }

        int getHeight(AVLNode *node) const {
            if (node == nullptr) return 0;
            return node->height;
        }

        void heightUpdate(AVLNode *node) {
            if (node == nullptr) return;
            node->height = std::max(getHeight(node->leftSon), getHeight(node->rightSon)) + 1;
        }

        int getFactor(AVLNode *node) const { // 获得平衡因子
            if (node == nullptr) return 0;
            return getHeight(node->leftSon) - getHeight(node->rightSon);
        }

        void link(AVLNode *node, AVLNode *sonNode, SideType side) { // link node & sonNode
            if (sonNode) sonNode->parent = node;
            if (node) {
                if (side == LEFT) node->leftSon = sonNode;
                else if (side == RIGHT) node->rightSon = sonNode;
                else {
                    //side == NULL_SIDE
                    return;
                }
            }
        }

        /*
         * rotate and fix
         */

        void LL(AVLNode *node) { // Node 左儿子向上, 右旋
            AVLNode *sonNode = node->leftSon;
            AVLNode *parentNode = node->parent;

            if (node == nullptr || sonNode == nullptr) return;

            if (root == node) root = node->leftSon;

            link(parentNode, sonNode, getSide(node));
            link(node, sonNode->rightSon, LEFT);
            link(sonNode, node, RIGHT);

            heightUpdate(node);
            heightUpdate(sonNode);
            heightUpdate(parentNode);
        }

        void RR(AVLNode *node) { //Node 右儿子向上, 左旋
            AVLNode *sonNode = node->rightSon;
            AVLNode *parentNode = node->parent;

            if (node == nullptr || sonNode == nullptr) return;

            if (root == node) root = node->rightSon;

            link(parentNode, sonNode, getSide(node));
            link(node, sonNode->leftSon, RIGHT);
            link(sonNode, node, LEFT);

            heightUpdate(node);
            heightUpdate(sonNode);
            heightUpdate(parentNode);
        }

        void fix(AVLNode *node) { // 修改该节点平衡, 进行一次或两次旋转
            if (node == nullptr) return;

            int factor = getFactor(node);
            AVLNode *parentNode = node->parent;

            if (factor < -1 || factor > 1) {
                DEBUG("fix " << node)

                if (factor > 0) { // L-
                    if (getFactor(node->leftSon) > 0) {
                        DEBUG("LL")
                        LL(node); // LL
                    }
                    else {
                        DEBUG("LR")
                        RR(node->leftSon);
                        LL(node);
                    }
                } else { // R-
                    if (getFactor(node->rightSon) < 0) {
                        DEBUG("RR")
                        RR(node); // RR
                    }
                    else {
                        DEBUG("RL")
                        LL(node->rightSon);
                        RR(node);
                    }
                }
            }

            heightUpdate(node);
            fix(parentNode);
        }

    public:
        AVLTree():siz(0), root(nullptr) {}

        ~AVLTree() {
            if (root != nullptr) delete root;
        }

        bool insert(const Key& key, const Val& val) {
            AVLNode *node = root, *newNode = new AVLNode(key, val);
            if (node == nullptr) {
                root = newNode;
                return true;
            }
            while (true) {
                if (Compare()(key, node->key)) {
                    if (node->leftSon == nullptr) {
                        link(node, newNode, LEFT);
                        break;
                    }
                    node = node->leftSon;
                }
                else if (Compare()(node->key, key)) {
                    if (node->rightSon == nullptr) {
                        link(node, newNode, RIGHT);
                        break;
                    }
                    node = node->rightSon;
                }
                else {
                    DEBUG("key duplicate!")
                    delete newNode;
                    return false;
                }
            }

            fix(newNode);
            return true;
        }

        bool find(const Key& key, Val& val) const {
            AVLNode *node = root;
            while (node != nullptr) {
                if (Compare()(key, node->key)) node = node->leftSon;
                else if (Compare()(node->key, key)) node = node->rightSon;
                else {
                    DEBUG("found!")
                    val = node->val;
                    return true;
                }
            }
            return false;
        }

        void display() const {
            root->display();
        }
    };
}

#endif //DS03_AVLTREE_AVLTREE_HPP
