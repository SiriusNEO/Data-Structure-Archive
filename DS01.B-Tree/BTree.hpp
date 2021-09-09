//
// Created by SiriusNEO on 2021/8/30.
//

#ifndef data_TREE_BTREE_HPP
#define data_TREE_BTREE_HPP

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <functional>

namespace Sirius {

    #define BOMB printf("bomb\n");
    #define DEBUG(_x) //std::cout << _x << '\n';

    /*
     * 文件上的B树
     */
    template<class Key, class Val, int M = 4> //Key - Value Pair, M为阶数
    class BTree {
        typedef int fpos_t; //约定文件上的位置均用 int32 表示
        typedef int hash_t; //key 值统一经过哈希, 类型约定为 int32 (-1表示不存在)
        static const int HASH_MOD = ((1<<30) - 1);
        static const fpos_t NULL_NUM = -1; //空文件位置
        static const int NODE_MIN_SIZE = (M + 1) / 2 - 1; //除根节点外, BTreeNode size下限

    private:
        /*
         * B树节点, 大体结构: (siz) son[0] data[0] son[1] data[1] .. son[M-2] data[M-2] son[M-1]
         * 关键字个数最大值M-1, son个数最大值M
         * 统一大小+1, 因为插入时会多; son由于头尾都有, 比K-V数量多一, 处理时候多多注意
         */
        struct BTreeNode {
            fpos_t parent;
            size_t siz;
            hash_t key[M]; //关键字
            Val val[M]; //数据位置
            fpos_t son[M + 1]; //子节点指针
            BTreeNode() : parent(NULL_NUM), siz(0) {
                for (int i = 0; i < M; ++i) key[i] = NULL_NUM;
                for (int i = 0; i < M + 1; ++i) son[i] = NULL_NUM;
            }
        };

        /*
         *  内存回收记录，是一个栈
         */
        template<int LEN = 10>
        struct RecyclePool {
            fpos_t pool[LEN + 1];
            size_t tp;

            RecyclePool(): tp(0) {
                for (int i = 0; i <= LEN; ++i) pool[i] = NULL_NUM;
            }

            void push(fpos_t freePos) {
                if (tp == LEN) { //满, 浪费
                    return;
                }
                for (int i = 1; i <= LEN; ++i)
                    if (pool[i] == freePos) return; // 不重复回收
                pool[++tp] = freePos;
            }

            fpos_t top() const {
                return pool[tp];
            }

            bool empty() const {
                return tp == 0;
            }

            void pop() {
                if (tp > 0) tp--;
            }
        };

        /*
         * B-Tree 主体: base + data 设计
         * base为树的基础, data为数据储存
         */

        struct TreeBase {
            fpos_t rootPos; //根节点
            size_t siz;
            RecyclePool<2002> recyclePool;

            explicit TreeBase(fpos_t _rootPos): siz(0), rootPos(_rootPos), recyclePool() {
            }
        } base;

        FILE *data;

        /*
         * 内部函数, 获取一个内存空位, 用于开一块新的BTreeNode
         * 如果回收池里有就获取, 没有就返回末位置, alloc会不断往后计数, 内存回收使得其不会浪费
         * 注意一开始的root位置相当于已分配, 所以计数器从1开始
         */
        fpos_t newFilePos() {
            static int allocCounter = 0;
            if (base.recyclePool.empty()) {
                allocCounter++;
                return sizeof(TreeBase) + allocCounter * sizeof(BTreeNode);
            }
            fpos_t ret = base.recyclePool.top();
            base.recyclePool.pop();
            return ret;
        }

        /*
         * 内部函数, 显示一个BTreeNode信息, 并且递归到其子节点
         */
        void nodeDisplay(fpos_t nodePos) const {
            if (nodePos == NULL_NUM) return;
            BTreeNode node;
            fseek(data, nodePos, SEEK_SET);
            fread(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
            printf("\n* Node stored in %d *\n", nodePos);
            printf("size: %lu\n", node.siz);
            printf("parent: %d\n", node.parent);
            printf("key: ");
            for (int i = 0; i < node.siz; ++i)
                printf("%d ", node.key[i]);
            printf("\nval: ");
            for (int i = 0; i < node.siz; ++i)
                std::cout << node.val[i] << " ";
            printf("\nson: ");
            for (int i = 0; i < node.siz + 1; ++i)
                printf("%d ", node.son[i]);
            printf("\n");
            for (int i = 0; i < node.siz + 1; ++i)
                nodeDisplay(node.son[i]);
        }

        /*
         * 内部函数, 在一个BTreeNode里插入一个K-V, 如果数量>M则分裂并上提中间元素, 递归父亲nodeInsert
         * 注意递归到根节点的处理, 注意son位置的修改
         */
        void nodeInsert(BTreeNode &node, fpos_t nodePos, int insertPos,
                        fpos_t keyHash, const Val& val, fpos_t sonPos) {
            //son[ip-1] key[ip-1] son[ip] key[ip] ...
            //[insertPos, node.siz) 位移到 [insertPos+1, node.siz+1), 新节点插入在insertPos
            node.siz++;
            for (int i = node.siz - 1; i > insertPos; --i) {
                node.key[i] = node.key[i - 1];
                node.val[i] = node.val[i - 1];
                node.son[i + 1] = node.son[i];
            }
            node.key[insertPos] = keyHash;
            node.val[insertPos] = val;
            node.son[insertPos + 1] = sonPos;

            //如果已经满, 考虑分裂
            if (node.siz >= M) {
                int mid = node.siz / 2; //mid 上提
                hash_t midKeyHash = node.key[mid];
                Val midVal = node.val[mid];

                DEBUG("mid up K-V: " << midKeyHash << " " << midVal)

                //分裂, 新节点转移 [mid+1, node.siz) 部分的数据, 原数据清空
                BTreeNode newNode;
                fpos_t newNodePos = newFilePos();
                newNode.parent = node.parent;

                for (int i = mid + 1; i < node.siz; ++i) {
                    newNode.siz++;
                    newNode.key[i - mid - 1] = node.key[i];
                    newNode.val[i - mid - 1] = node.val[i];
                    newNode.son[i - mid] = node.son[i + 1];
                    if (node.son[i+1] != NULL_NUM) {
                        DEBUG("modified son: " << node.son[i+1])
                        fseek(data, node.son[i + 1], SEEK_SET);
                        fwrite(reinterpret_cast<char *>(&newNodePos), sizeof(fpos_t), 1, data); //直接写入parent位置
                    }
                    node.key[i] = node.son[i + 1] = NULL_NUM;
                }
                node.siz -= newNode.siz;

                //son 数组多一个处理
                newNode.son[0] = node.son[mid + 1];
                if (node.son[mid + 1] != NULL_NUM) {
                    fseek(data, node.son[mid + 1], SEEK_SET);
                    fwrite(reinterpret_cast<char *>(&newNodePos), sizeof(fpos_t), 1, data);
                }
                node.son[mid + 1] = NULL_NUM;

                //mid 在原块删除
                node.siz--;
                node.key[mid] = NULL_NUM;

                if (node.parent == NULL_NUM) {
                    //如果当前节点是根节点, 创造新根节点
                    //son[0] key[0] son[1]
                    BTreeNode newRoot;
                    fpos_t newRootPos = newFilePos();
                    DEBUG("nodePos: " << nodePos << " newNodePos: " << newNodePos << " newRootPos: " << newRootPos)
                    newRoot.siz = 1;
                    newRoot.key[0] = midKeyHash;
                    newRoot.val[0] = midVal;
                    newRoot.son[0] = nodePos;
                    newRoot.son[1] = newNodePos;
                    node.parent = newNode.parent = newRootPos;
                    fseek(data, newRootPos, SEEK_SET);
                    fwrite(reinterpret_cast<char *>(&newRoot), sizeof(BTreeNode), 1, data);
                    base.rootPos = newRootPos; //换根
                    fseek(data, nodePos, SEEK_SET);
                    fwrite(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
                    fseek(data, newNodePos, SEEK_SET);
                    fwrite(reinterpret_cast<char *>(&newNode), sizeof(BTreeNode), 1, data);
                    return;
                } else {
                    //否则将mid插入父节点
                    BTreeNode parentNode;
                    fseek(data, node.parent, SEEK_SET);
                    fread(reinterpret_cast<char *>(&parentNode), sizeof(BTreeNode), 1, data);
                    int i = std::lower_bound(parentNode.key, parentNode.key+parentNode.siz, midKeyHash) - parentNode.key;
                    fseek(data, nodePos, SEEK_SET);
                    fwrite(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
                    fseek(data, newNodePos, SEEK_SET);
                    fwrite(reinterpret_cast<char *>(&newNode), sizeof(BTreeNode), 1, data);
                    nodeInsert(parentNode, node.parent, i, midKeyHash, midVal, newNodePos); //一定找得到
                    return;
                }
            }
            fseek(data, nodePos, SEEK_SET);
            fwrite(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
        }

        /*
         * 内部函数, 个数调整, 会一直往上递归
         */
        void deleteFix(BTreeNode &node, fpos_t nodePos) {
            //根节点无MIN_SIZE限制
            if (node.siz >= NODE_MIN_SIZE || nodePos == base.rootPos) return;

            BTreeNode parentNode, leftBro, rightBro;
            fseek(data, node.parent, SEEK_SET);
            fread(reinterpret_cast<char *>(&parentNode), sizeof(BTreeNode), 1, data);
            //stupid find bro method
            //son[0] key[0] son[1] key[1] ...
            for (int i = 0; i < parentNode.siz + 1; ++i) {
                if (parentNode.son[i] == nodePos) {
                    if (i > 0) {
                        fseek(data, parentNode.son[i - 1], SEEK_SET);
                        fread(reinterpret_cast<char *>(&leftBro), sizeof(BTreeNode), 1, data);
                    }
                    if (i < parentNode.siz) {
                        fseek(data, parentNode.son[i + 1], SEEK_SET);
                        fread(reinterpret_cast<char *>(&rightBro), sizeof(BTreeNode), 1, data);
                    }

                    //borrow 均为 K-V 值交换, 不改变树的结构, 不用改parent
                    //merge 需要改

                    if (leftBro.siz > NODE_MIN_SIZE) { // borrow from left
                        //left key[i-1] node
                        DEBUG("left borrow")
                        for (int j = node.siz; j > 0; --j) {
                            node.key[j] = node.key[j-1];
                            node.val[j] = node.val[j-1];
                            node.son[j+1] = node.son[j];
                        }
                        node.son[1] = node.son[0];

                        node.key[0] = parentNode.key[i - 1];
                        node.val[0] = parentNode.val[i - 1];
                        node.son[0] = leftBro.son[leftBro.siz];
                        node.siz++;

                        parentNode.key[i - 1] = leftBro.key[leftBro.siz - 1];
                        parentNode.val[i - 1] = leftBro.val[leftBro.siz - 1];
                        leftBro.key[leftBro.siz - 1] = leftBro.son[leftBro.siz] = NULL_NUM;
                        leftBro.siz--;

                        fseek(data, parentNode.son[i-1], SEEK_SET);
                        fwrite(reinterpret_cast<char *>(&leftBro), sizeof(BTreeNode), 1, data);
                        fseek(data, node.parent, SEEK_SET);
                        fwrite(reinterpret_cast<char *>(&parentNode), sizeof(BTreeNode), 1, data);
                        fseek(data, nodePos, SEEK_SET);
                        fwrite(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
                    } else if (rightBro.siz > NODE_MIN_SIZE) { // borrow from right
                        //node key[i] right
                        DEBUG("right borrow")
                        node.key[node.siz] = parentNode.key[i];
                        node.val[node.siz] = parentNode.val[i];
                        node.son[node.siz + 1] = rightBro.son[0];
                        node.siz++;

                        parentNode.key[i] = rightBro.key[0];
                        parentNode.val[i] = rightBro.val[0];

                        rightBro.son[0] = rightBro.son[1];
                        for (int j = 0; j < rightBro.siz-1; j++) {
                            rightBro.key[j] = rightBro.key[j + 1];
                            rightBro.val[j] = rightBro.val[j + 1];
                            rightBro.son[j + 1] = rightBro.son[j + 2];
                        }

                        rightBro.key[rightBro.siz-1] = rightBro.son[rightBro.siz] = NULL_NUM;
                        rightBro.siz--;

                        fseek(data, parentNode.son[i+1], SEEK_SET);
                        fwrite(reinterpret_cast<char *>(&rightBro), sizeof(BTreeNode), 1, data);
                        fseek(data, node.parent, SEEK_SET);
                        fwrite(reinterpret_cast<char *>(&parentNode), sizeof(BTreeNode), 1, data);
                        fseek(data, nodePos, SEEK_SET);
                        fwrite(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
                    } else { // merge
                        if (leftBro.siz > 0) {
                            DEBUG("left merge")
                            //left key[i-1] node, node -> left
                            leftBro.key[leftBro.siz] = parentNode.key[i-1];
                            leftBro.val[leftBro.siz] = parentNode.val[i-1];
                            leftBro.son[leftBro.siz + 1] = node.son[0];
                            if (node.son[0] != NULL_NUM) {
                                fseek(data, node.son[0], SEEK_SET);
                                fwrite(reinterpret_cast<char *>(&parentNode.son[i-1]), sizeof(fpos_t), 1, data);
                            }
                            for (int j = 0; j < node.siz; ++j) {
                                leftBro.key[leftBro.siz + 1 + j] = node.key[j];
                                leftBro.val[leftBro.siz + 1 + j] = node.val[j];
                                leftBro.son[leftBro.siz + 1 + j + 1] = node.son[j + 1];
                                if (node.son[j+1] != NULL_NUM) {
                                    fseek(data, node.son[j+1], SEEK_SET);
                                    fwrite(reinterpret_cast<char *>(&parentNode.son[i-1]), sizeof(fpos_t), 1, data);
                                }
                            }
                            leftBro.siz += node.siz + 1;
                            //left key[i-1] node key[i] son[i+1], delete key[i-1]
                            for (int j = i; j < parentNode.siz; ++j) {
                                parentNode.key[j-1] = parentNode.key[j];
                                parentNode.val[j-1] = parentNode.val[j];
                                parentNode.son[j] = parentNode.son[j+1];
                            }
                            parentNode.siz--;
                            parentNode.key[parentNode.siz] = -1;
                            base.recyclePool.push(nodePos); //delete node
                            if (parentNode.siz <= 0) { //父节点删空, 减少一层
                                base.recyclePool.push(node.parent);
                                leftBro.parent = parentNode.parent;
                                if (parentNode.parent != NULL_NUM) {
                                    BTreeNode grandpaNode;
                                    fseek(data, parentNode.parent, SEEK_SET);
                                    fread(reinterpret_cast<char *>(&grandpaNode), sizeof(BTreeNode), 1, data);
                                    for (int j = 0; j < grandpaNode.siz + 1; ++j)
                                        if (grandpaNode.son[j] == node.parent) {
                                            grandpaNode.son[j] = parentNode.son[i-1];
                                            fseek(data, parentNode.parent, SEEK_SET);
                                            fwrite(reinterpret_cast<char *>(&grandpaNode), sizeof(BTreeNode), 1, data);
                                            break;
                                        }
                                } else {
                                    base.rootPos = parentNode.son[i-1];
                                }
                            } else {
                                fseek(data, node.parent, SEEK_SET);
                                fwrite(reinterpret_cast<char *>(&parentNode), sizeof(BTreeNode), 1, data);
                            }
                            fseek(data, parentNode.son[i-1], SEEK_SET);
                            fwrite(reinterpret_cast<char *>(&leftBro), sizeof(BTreeNode), 1, data);
                            deleteFix(parentNode, node.parent);
                        } else {
                            DEBUG("right merge")
                            //node key[i] right, right -> node
                            node.key[node.siz] = parentNode.key[i];
                            node.val[node.siz] = parentNode.val[i];
                            node.son[node.siz + 1] = rightBro.son[0];
                            if (rightBro.son[0] != NULL_NUM) {
                                fseek(data, rightBro.son[0], SEEK_SET);
                                fwrite(reinterpret_cast<char *>(&nodePos), sizeof(fpos_t), 1, data);
                            }

                            for (int j = 0; j < rightBro.siz; ++j) {
                                node.key[node.siz + 1 + j] = rightBro.key[j];
                                node.val[node.siz + 1 + j] = rightBro.val[j];
                                node.son[node.siz + 1 + j + 1] = rightBro.son[j + 1];
                                if (rightBro.son[j+1] != NULL_NUM) {
                                    fseek(data, rightBro.son[j+1], SEEK_SET);
                                    fwrite(reinterpret_cast<char *>(&nodePos), sizeof(fpos_t), 1, data);
                                }
                            }
                            node.siz += rightBro.siz + 1;
                            fseek(data, nodePos, SEEK_SET);
                            fwrite(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
                            //node key[i] right key[i+1], delete key

                            base.recyclePool.push(parentNode.son[i+1]); //delete right

                            for (int j = i+1; j < parentNode.siz; ++j) {
                                parentNode.key[j-1] = parentNode.key[j];
                                parentNode.val[j-1] = parentNode.val[j];
                                parentNode.son[j] = parentNode.son[j+1];
                            }
                            parentNode.siz--;
                            parentNode.key[parentNode.siz] = -1;

                            if (parentNode.siz <= 0) { //父节点删空, 减少一层
                                base.recyclePool.push(node.parent);
                                node.parent = parentNode.parent;
                                if (parentNode.parent != NULL_NUM) {
                                    BTreeNode grandpaNode;
                                    fseek(data, parentNode.parent, SEEK_SET);
                                    fread(reinterpret_cast<char *>(&grandpaNode), sizeof(BTreeNode), 1, data);
                                    for (int j = 0; j < grandpaNode.siz + 1; ++j)
                                        if (grandpaNode.son[j] == node.parent) {
                                            grandpaNode.son[j] = nodePos;
                                            fseek(data, parentNode.parent, SEEK_SET);
                                            fwrite(reinterpret_cast<char *>(&grandpaNode), sizeof(BTreeNode), 1, data);
                                            break;
                                        }
                                } else {
                                    base.rootPos = parentNode.son[i];
                                }
                            } else {
                                fseek(data, node.parent, SEEK_SET);
                                fwrite(reinterpret_cast<char *>(&parentNode), sizeof(BTreeNode), 1, data);
                            }
                            deleteFix(parentNode, node.parent);
                        }
                    }
                    break;
                }
            }
        }

        /*
         * 内部函数, 删除指定位置的节点
         * 一定是最底层, son均为-1, 不用操作, fix交给专门函数做
         */
        void nodeDelete(BTreeNode &node, fpos_t nodePos, int deletePos) {
            node.siz--;
            DEBUG("deletePos: " << deletePos)
            for (int i = deletePos; i < node.siz; ++i) {
                node.key[i] = node.key[i + 1];
                node.val[i] = node.val[i + 1];
            }
            node.key[node.siz] = -1;

            deleteFix(node, nodePos);

            if (base.rootPos == nodePos && node.siz == 0) {
                base.rootPos = sizeof(TreeBase);
            }

            fseek(data, nodePos, SEEK_SET);
            fwrite(reinterpret_cast<char *>(&node), sizeof(BTreeNode), 1, data);
        }

    public:
        /*
         * 采用单文件设计, 便于内存回收
         * 索引-数据库架构可以将此B树作为索引, 另写一个文件池结合搭建
         */
        explicit BTree(const char *dataFileName):base(sizeof(TreeBase)) {
            data = fopen(dataFileName, "rb+");
            if (!data) {
                FILE *fileCreator;
                fileCreator = fopen(dataFileName, "wb");
                fwrite(reinterpret_cast<char *>(&base), sizeof(TreeBase), 1, fileCreator);
                fclose(fileCreator);
                data = fopen(dataFileName, "rb+");
                fseek(data, 0, SEEK_SET);
            } else {
                DEBUG("second time")
                fseek(data, 0, SEEK_SET);
                fread(reinterpret_cast<char *>(&base), sizeof(TreeBase), 1, data);
            }
        }

        ~BTree() {
            //析构时注意写回base
            fseek(data, 0, SEEK_SET);
            fwrite(reinterpret_cast<char *>(&base), sizeof(TreeBase), 1, data);
            fclose(data);
        }

        void display() const {
            printf("\n* --- BTree (%d level) --- *\n", M);
            printf("size: %lu\n", base.siz);
            printf("base size: %lu\n", sizeof(TreeBase));
            printf("node size: %lu\n", sizeof(BTreeNode));
            if (base.siz > 0) {
                printf("rootPos: %d\n", base.rootPos);
                nodeDisplay(base.rootPos);
            } else {
                printf("<empty tree>\n");
            }
        }

        /*
         * 插入: 从根开始, 到最底层的节点 (子节点是nullptr) 插入
         * 只负责从根开始往下找到最底层节点, 插入的递归交给内部函数 nodeInsert
         * 返回是否插入成功
         */
        bool insert(const Key &key, const Val &val) {
            hash_t keyHash = std::hash<Key>{}(key) % HASH_MOD;
            BTreeNode nowNode;
            fseek(data, base.rootPos, SEEK_SET);
            fpos_t nowNodePos = ftell(data);

            if (base.siz != 0) { //为空则不读, 直接调用默认构造函数
                fread(reinterpret_cast<char *>(&nowNode), sizeof(BTreeNode), 1, data);
            }

            while (true) {
                int i = std::lower_bound(nowNode.key, nowNode.key+nowNode.siz, keyHash) - nowNode.key;
                if (nowNode.key[i] == keyHash) {
                    DEBUG("key duplicate")
                    return false;
                }
                if (nowNode.son[i] == NULL_NUM) { //最后一层, i与后面后移
                    DEBUG("insert K-V pair: " << keyHash << " " << val)
                    nodeInsert(nowNode, nowNodePos, i, keyHash, val, NULL_NUM);
                    base.siz++;
                    return true;
                }
                nowNodePos = nowNode.son[i];
                fseek(data, nowNode.son[i], SEEK_SET);
                fread(reinterpret_cast<char*>(&nowNode), sizeof(BTreeNode), 1, data); //往下走
            }
        }

        /*
         * 查询: 从根向下遍历
         * 返回: 是否找到, 值的返回采用引用的方式提高效率
         */
        bool find(const Key &key, Val &val) const {
            hash_t keyHash = std::hash<Key>{}(key) % HASH_MOD;
            BTreeNode nowNode;
            fseek(data, base.rootPos, SEEK_SET);

            if (base.siz == 0) {
                return false;
            }

            fread(reinterpret_cast<char *>(&nowNode), sizeof(BTreeNode), 1, data);
            while (true) {
                int i = std::lower_bound(nowNode.key, nowNode.key+nowNode.siz, keyHash) - nowNode.key;
                if (nowNode.key[i] == keyHash) {
                    val = nowNode.val[i]; //found
                    return true;
                }
                if (nowNode.son[i] == NULL_NUM) { //最后一层, 找不到
                    return false;
                }
                fseek(data, nowNode.son[i], SEEK_SET);
                fread(reinterpret_cast<char*>(&nowNode), sizeof(BTreeNode), 1, data); //往下走
            }
        }

        /*
         * 修改: 基本同查询, 找到值后修改即可
         * 返回: 是否修改成功
         */
        bool modify(const Key &key, const Val &val) {
            hash_t keyHash = std::hash<Key>{}(key) % HASH_MOD;
            BTreeNode nowNode;
            fseek(data, base.rootPos, SEEK_SET);
            fpos_t nowNodePos = ftell(data);

            if (base.siz == 0) {
                return false;
            }

            fread(reinterpret_cast<char *>(&nowNode), sizeof(BTreeNode), 1, data);
            while (true) {
                int i = std::lower_bound(nowNode.key, nowNode.key+nowNode.siz, keyHash) - nowNode.key;
                if (nowNode.key[i] == keyHash) {
                    nowNode.val[i] = val;
                    fseek(data, nowNodePos, SEEK_SET);
                    fwrite(reinterpret_cast<char*>(&nowNode), sizeof(BTreeNode), 1, data);
                    return true;
                }
                if (nowNode.son[i] == NULL_NUM) { //最后一层, 找不到
                    return false;
                }
                nowNodePos = nowNode.son[i];
                fseek(data, nowNode.son[i], SEEK_SET);
                fread(reinterpret_cast<char*>(&nowNode), sizeof(BTreeNode), 1, data); //往下走
            }
        }

        /*
         * 删除: 基本同查询, 找到值后调用内部函数删除
         * 返回: 是否删除成功 (即是否找到)
         */
        bool del(const Key &key) {
            hash_t keyHash = std::hash<Key>{}(key) % HASH_MOD;
            BTreeNode nowNode;
            fseek(data, base.rootPos, SEEK_SET);
            fpos_t nowNodePos = ftell(data);

            if (base.siz <= 0) {
                DEBUG("empty tree")
                return false;
            }

            fread(reinterpret_cast<char *>(&nowNode), sizeof(BTreeNode), 1, data);
            while (true) {
                int i = std::lower_bound(nowNode.key, nowNode.key+nowNode.siz, keyHash) - nowNode.key;
                if (nowNode.key[i] == keyHash) {
                    //key[i] son[i+1]
                    if (nowNode.son[i+1] != NULL_NUM) { //非最后一层
                        BTreeNode targetNode;
                        int targetNodePos = nowNode.son[i+1];
                        fseek(data, nowNode.son[i+1], SEEK_SET);
                        fread(reinterpret_cast<char*>(&targetNode), sizeof(BTreeNode), 1, data);
                        while (targetNode.son[0] != NULL_NUM) { //查后继
                            targetNodePos = targetNode.son[0];
                            fseek(data, targetNode.son[0], SEEK_SET);
                            fread(reinterpret_cast<char*>(&targetNode), sizeof(BTreeNode), 1, data);
                        }
                        nowNode.key[i] = targetNode.key[0];
                        nowNode.val[i] = targetNode.val[0];
                        fseek(data, nowNodePos, SEEK_SET);
                        fwrite(reinterpret_cast<char*>(&nowNode), sizeof(BTreeNode), 1, data);
                        nodeDelete(targetNode, targetNodePos, 0);
                    }
                    else {
                        nodeDelete(nowNode, nowNodePos, i);
                    }
                    base.siz--;
                    return true;
                }
                if (nowNode.son[i] == NULL_NUM) { //最后一层, 找不到
                    return false;
                }
                nowNodePos = nowNode.son[i];
                fseek(data, nowNode.son[i], SEEK_SET);
                fread(reinterpret_cast<char*>(&nowNode), sizeof(BTreeNode), 1, data); //往下走
            }
        }
    };
}

#endif //data_TREE_BTREE_HPP
