//
// Created by SiriusNEO on 2021/9/15.
//

#ifndef DS05_BINOMIALHEAP_BINOMIALHEAP_HPP
#define DS05_BINOMIALHEAP_BINOMIALHEAP_HPP

#include <vector>
#include <functional>
#include <iostream>
#include <cassert>
#include <memory>

namespace Sirius {

    #define DEBUG(_x) //std::cout << _x << '\n';

    /*
     * 二项堆, 由许多二项树构成
     */
    template<class T, class Compare=std::less<T>>
    class BinomialHeap {

        static const int LEVEL_INF = 100002;

        public:

            // 使用宏定义简化类型
            // 不使用 typedef 是为了避免类型声明先后问题
            #define NodeCur std::shared_ptr<Node>
            #define TreeCur std::shared_ptr<BinomialTree>
            
            // 私有成员大多是 static 的工具函数

            /*
             * 节点部分
             */
        
            struct Node {
                T data;
                std::vector<NodeCur> son;

                explicit Node(const T& _data):data(_data) {}

                void display() {
                    std::cout << "* --- Node in " << this << " --- *\n";
                    std::cout << "data: " << data << "\n";
                    std::cout << "son: ";
                    for (auto i : son) std::cout << i << ' ';
                    std::cout << "\n";
                    for (auto i : son) i->display();
                }
            };
            
            /*
             * 二项树部分
             */

            struct BinomialTree {
                int level; // 阶数, 即 Bk 里的 k
                NodeCur root;
                TreeCur pre, nxt; // 用链表形式连接
                
                explicit BinomialTree(const T& _data):level(0), pre(nullptr), nxt(nullptr) {
                    root = std::make_shared<Node> (_data);
                }

                BinomialTree(int _level, NodeCur _root):level(_level), root(_root), pre(nullptr), nxt(nullptr) {}

                void display() {
                    std::cout << "\n* --- Binomial Tree in " << this << " --- *\n";
                    std::cout << "level: " << level << '\n';
                    std::cout << "root: " << root << '\n';
                    root->display();
                }
            };

            size_t siz;
            TreeCur treeListHead;

            /*
             * 合并两棵同阶树, 返回新树
             * 注意合并后只保留一棵树, 另一棵被delete
             */
            static TreeCur treeMerge(TreeCur tree1, TreeCur tree2) {
                if (tree1 == nullptr) return tree2;
                if (tree2 == nullptr) return tree1;

                TreeCur mainTree = nullptr, subTree = nullptr; // 将subTree合并到mainTree上, 删除subTree
                
                if (Compare()(tree1->root->data, tree2->root->data)) {
                    mainTree = tree1, subTree = tree2;
                } else {
                    mainTree = tree2, subTree = tree1;
                }

                mainTree->level++;
                mainTree->root->son.push_back(subTree->root); 
                
                return mainTree;
            }

            static inline int getLevel(TreeCur tree) { // 空树被解释为无穷大, 以便处理
                if (tree == nullptr) return LEVEL_INF;
                return tree->level;
            }

            static inline TreeCur getNxt(TreeCur tree) { // 同样处理空的情况
                if (tree == nullptr) return nullptr;
                return tree->nxt; 
            }

            /*
             * 往给定根表中添加一个元素
             * 注意要传递链表头与链表尾的引用, 头用于新开根表时赋值, 尾用于每次更新
             * 其实是本人不想写太多重复部分, 就粗暴地封装在一起了
             */
            static void append(TreeCur &treeListHead, TreeCur &treeListTail, TreeCur newTree) {
                if (treeListHead == nullptr && treeListTail == nullptr) {
                    treeListHead = treeListTail = newTree;
                    return;
                }
                assert(treeListTail != nullptr);
                treeListTail->nxt = newTree;
                newTree->pre = treeListTail;
                treeListTail = newTree;
            }

        public:
            BinomialHeap():treeListHead(nullptr), siz(0) {}

            BinomialHeap(TreeCur _treeListHead, size_t _siz):treeListHead(_treeListHead), siz(_siz) {}

            explicit BinomialHeap(const T& _data):siz(1) {
                treeListHead = std::make_shared<BinomialTree>(_data);
            }

            /*
             * 合并, 将 other 合并入当前堆, 并置空 other
             * 使用shared_ptr, 不用考虑内存托管情况
             */
            void merge(BinomialHeap& other) {
                siz += other.siz;

                TreeCur thisTree = treeListHead;
                TreeCur otherTree = other.treeListHead;
                TreeCur carryTree = nullptr;
                TreeCur newTreeListTail = nullptr;
                
                treeListHead = nullptr;

                while (thisTree != nullptr || otherTree != nullptr || carryTree != nullptr) {

                    // 由于 treeMerge 操作可能会删除原指针, 先存下下一个位置以便最后更新
                    TreeCur thisTreeNxt = getNxt(thisTree);
                    DEBUG("this nxt: " << thisTreeNxt)
                    TreeCur otherTreeNxt = getNxt(otherTree);
                    DEBUG("other nxt: " << otherTreeNxt)
                    
                    int thisLevel = getLevel(thisTree);
                    int otherLevel = getLevel(otherTree);
                    int carryLevel = getLevel(carryTree);
                    
                    DEBUG("merging... " << "this: " << thisTree << " other: " << otherTree << " carry: " << carryTree)
                    DEBUG("level... " << "this: " << thisLevel << " other: " << otherLevel << " carry: " << carryLevel)

                    if (carryTree == nullptr || (carryLevel > thisLevel && carryLevel > otherLevel)) {
                        if (thisLevel < otherLevel) {
                            append(treeListHead, newTreeListTail, thisTree);
                            thisTree = thisTreeNxt;
                        } 
                        else if (thisLevel > otherLevel) {
                            append(treeListHead, newTreeListTail, otherTree);
                            otherTree = otherTreeNxt;
                        } 
                        else { //thisLevel == otherLevel
                            carryTree = treeMerge(thisTree, otherTree);        
                            thisTree = thisTreeNxt;
                            otherTree = otherTreeNxt;
                        }
                    } else {
                        if (carryLevel < thisLevel && carryLevel < otherLevel) {
                            append(treeListHead, newTreeListTail, carryTree);
                            carryTree = nullptr;
                        } 
                        else if (carryLevel == thisLevel && thisLevel < otherLevel) {
                            carryTree = treeMerge(thisTree, carryTree);
                            thisTree = thisTreeNxt;
                        }
                        else if (carryLevel == otherLevel && otherLevel < thisLevel) {
                            carryTree = treeMerge(otherTree, carryTree);
                            otherTree = otherTreeNxt;
                        }
                        else if (carryLevel == otherLevel && thisLevel == otherLevel) {
                            append(treeListHead, newTreeListTail, carryTree);
                            carryTree = treeMerge(thisTree, otherTree);        
                            thisTree = thisTreeNxt;
                            otherTree = otherTreeNxt;
                        }
                    }
                }
            }

            void push(const T& data) {
                BinomialHeap newHeap(data);
                merge(newHeap);
            }

            T top() const {
                TreeCur tree = treeListHead;
                if (tree == nullptr) {
                    //empty
                    throw "Heap is Empty!\n";
                }
                T ret = tree->root->data;
                while (tree->nxt != nullptr) {
                    if (Compare()(tree->nxt->root->data, ret)) 
                        ret = tree->nxt->root->data;
                    tree = tree->nxt;
                }
                return ret;
            }

            void pop() {
                TreeCur tree = treeListHead;
                if (tree == nullptr) {
                    //empty
                    throw "Heap is Empty!\n";
                }
                TreeCur minTree = tree;
                while (tree->nxt != nullptr) {
                    if (Compare()(tree->nxt->root->data, minTree->root->data)) 
                        minTree = tree;
                    tree = tree->nxt;
                }

                // 从原根表删除
                if (minTree->pre != nullptr) {
                    minTree->pre->nxt = minTree->nxt;
                }
                if (minTree->nxt != nullptr) {
                    minTree->nxt->pre = minTree->pre;
                }
                if (minTree == treeListHead) treeListHead = minTree->nxt;

                // 根节点的 son-vector 阶数递增
                TreeCur newTreeListHead = nullptr, newTreeListTail = nullptr;

                for (int i = 0; i < minTree->root->son.size(); ++i) {
                    TreeCur sonTree = std::make_shared<BinomialTree>(i, minTree->root->son[i]);
                    append(newTreeListHead, newTreeListTail, sonTree);
                }

                BinomialHeap newHeap(newTreeListHead, (1<<minTree->root->son.size())-1);

                merge(newHeap);
            }

            void display() {
                std::cout << "\n* --- Binomial Heap --- *\n";
                std::cout << "siz: " << siz << '\n';
                std::cout << "treeList: \n";
                
                TreeCur tree = treeListHead;

                if (tree == nullptr) {
                    std::cout << "<empty>\n";
                    return;
                }

                while (tree != nullptr) {
                    tree->display();
                    tree = tree->nxt;
                }
            }
    };
}

#endif //DS05_BINOMIALHEAP_BINOMIALHEAP_HPP
