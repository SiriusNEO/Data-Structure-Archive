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
                
                explicit BinomialTree(const T& _data):level(0) {
                    root = std::make_shared<Node> (_data);
                }

                BinomialTree(int _level, NodeCur _root):level(_level), root(_root) {}

                void display() {
                    std::cout << "\n* --- Binomial Tree in " << this << " --- *\n";
                    std::cout << "level: " << level << '\n';
                    std::cout << "root: " << root << '\n';
                    root->display();
                }
            };

            size_t siz;
            std::vector<TreeCur> treeList;

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
            BinomialHeap():siz(0) {}

            BinomialHeap(const std::vector<TreeCur>& _treeList, size_t _siz):siz(), treeList(_treeList) {}

            explicit BinomialHeap(const T& _data):siz(1) {
                treeList.push_back(std::make_shared<BinomialTree>(_data));
            }

            /*
             * 合并, 将 other 合并入当前堆, 并置空 other
             * 使用shared_ptr, 不用考虑内存托管情况
             */
            void merge(BinomialHeap& other) {
                siz += other.siz;

                int thisTreePtr = 0;
                int otherTreePtr = 0;
                TreeCur carryTree = nullptr;
                
                std::vector<TreeCur> newTreeList;

                while (thisTreePtr < treeList.size() || otherTreePtr < other.treeList.size() || carryTree != nullptr) {
                    TreeCur thisTree = (thisTreePtr < treeList.size()) ? treeList[thisTreePtr] : nullptr;
                    TreeCur otherTree = (otherTreePtr < other.treeList.size()) ? other.treeList[otherTreePtr] : nullptr;

                    int thisLevel = getLevel(thisTree);
                    int otherLevel = getLevel(otherTree);
                    int carryLevel = getLevel(carryTree);
                    
                    DEBUG("merging... " << "this: " << thisTree << " other: " << otherTree << " carry: " << carryTree)
                    DEBUG("level... " << "this: " << thisLevel << " other: " << otherLevel << " carry: " << carryLevel)

                    if (carryTree == nullptr || (carryLevel > thisLevel || carryLevel > otherLevel)) {
                        if (thisLevel < otherLevel) {
                            newTreeList.push_back(thisTree);
                            ++thisTreePtr;
                        } 
                        else if (thisLevel > otherLevel) {
                            newTreeList.push_back(otherTree);
                            ++otherTreePtr;
                        } 
                        else { //thisLevel == otherLevel
                            assert(carryTree == nullptr);
                            carryTree = treeMerge(thisTree, otherTree);        
                            ++thisTreePtr;
                            ++otherTreePtr;
                        }
                    } else { // carryLevel <= thisLevel && carryLevel <= otherLevel
                        if (carryLevel < thisLevel && carryLevel < otherLevel) {
                            newTreeList.push_back(carryTree);
                            carryTree = nullptr;
                        } 
                        else if (carryLevel == thisLevel && thisLevel < otherLevel) {
                            carryTree = treeMerge(thisTree, carryTree);
                            ++thisTreePtr;
                        }
                        else if (carryLevel == otherLevel && otherLevel < thisLevel) {
                            carryTree = treeMerge(otherTree, carryTree);
                            ++otherTreePtr;
                        }
                        else if (carryLevel == otherLevel && thisLevel == otherLevel) {
                            newTreeList.push_back(carryTree);
                            carryTree = treeMerge(thisTree, otherTree);        
                            ++thisTreePtr;
                            ++otherTreePtr;
                        }
                    }
                }
                treeList = newTreeList;
            }

            size_t size() const {
                return siz;
            }

            void push(const T& data) {
                BinomialHeap newHeap(data);
                merge(newHeap);
            }

            bool empty() const {return siz == 0;}

            const T& top() const {
                TreeCur minTree;

                for (const TreeCur& tree : treeList) {
                    if (minTree == nullptr || Compare()(tree->root->data, minTree->root->data))
                        minTree = tree;
                }

                return minTree->root->data;
            }

            void pop() {
                TreeCur minTree;
                size_t newSize = siz - 1;

                for (const TreeCur& tree : treeList) {
                    if (minTree == nullptr || Compare()(tree->root->data, minTree->root->data))
                        minTree = tree;
                }

                // 从原根表删除
                for (int i = 0; i < treeList.size(); ++i) {
                    if (treeList[i] == minTree) {
                        for (int j = i + 1; j < treeList.size(); ++j)
                            treeList[j - 1] = treeList[j];
                        treeList.pop_back();    
                        break;    
                    }
                }    

                // 根节点的 son-vector 阶数递增
                std::vector<TreeCur> newTreeList;
                for (int i = 0; i < minTree->root->son.size(); ++i) {
                    TreeCur sonTree = std::make_shared<BinomialTree>(i, minTree->root->son[i]);
                    newTreeList.push_back(sonTree);
                }

                BinomialHeap newHeap(newTreeList, 0); // siz doesn't matter here

                merge(newHeap);

                siz = newSize;
            }

            void display() {
                std::cout << "\n* --- Binomial Heap --- *\n";
                std::cout << "siz: " << siz << '\n';
                std::cout << "treeList: \n";

                if (empty()) {
                    std::cout << "<empty>\n";
                    return;
                }

                for (const TreeCur& tree : treeList)
                    tree->display();
            }
    };
}

#endif //DS05_BINOMIALHEAP_BINOMIALHEAP_HPP
