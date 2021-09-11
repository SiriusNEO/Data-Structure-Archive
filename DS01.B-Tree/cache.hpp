//
// Created by SiriusNEO on 2021/9/9.
//

#ifndef DS01_B_TREE_CACHE_HPP
#define DS01_B_TREE_CACHE_HPP

#include <iostream>
#include <map>

namespace Sirius {
    #define BOMB printf("bomb\n");
    #define DEBUG(_x) //std::cout << _x << '\n';

    template <class Val, int LEN = 10>
    class LRUCache {
        typedef int fpos_t; //约定文件上的位置均用 int32 表示

        struct Node {
            fpos_t key;
            Val val;
            Node *pre;
            Node *nxt;

            Node(const fpos_t& _key, const Val& _val):key(_key), val(_val), pre(nullptr), nxt(nullptr) {}
        };

    private:
        FILE* file;

        size_t siz;
        std::map<fpos_t, Node*> table;
        Node *head, *tail;

        Node *pushFront(const fpos_t& key, const Val& val) {
            Node *newNode = new Node(key, val);
            if (siz == 0) {
                head = tail = newNode;
                siz++;
                return newNode;
            }
            newNode->nxt = head;
            if (newNode->nxt != nullptr) newNode->nxt->pre = newNode;
            head = newNode;
            siz++;
            return newNode;
        }

        void popBack() {
            Node *tmpTail = tail;

            table.erase(tail->key); //remove from table
            fseek(file, tail->key, SEEK_SET);
            fwrite(reinterpret_cast<char *>(&tail->val), sizeof(Val), 1, file); //write back
            tail = tail->pre;
            if (tail != nullptr) tail->nxt = nullptr;
            if (tmpTail != nullptr) delete tmpTail;
            siz--;
            if (siz == 0) head = nullptr;
        }

        void moveToFront(Node *node) {
            if (node == head) return;
            if (node->pre != nullptr)
                node->pre->nxt = node->nxt;
            if (node->nxt != nullptr)
                node->nxt->pre = node->pre;
            else
                tail = tail->pre;
            node->nxt = head;
            node->nxt->pre = node;
            node->pre = nullptr;
            head = node;
        }

        /*
         * 有则覆盖
         */
        void set(const fpos_t& key, const Val& val) {
            if (table.find(key) != table.end()) {
                moveToFront(table[key]);
                head->val = val;
                return;
            }
            table[key] = pushFront(key, val);
            if (siz > LEN) popBack();
        }

        bool get(const fpos_t& key, Val& val) {
            if (table.find(key) != table.end()) {
                moveToFront(table[key]);
                val = table[key]->val;
                return true;
            }
            return false;
        }

    public:

        LRUCache(): siz(0), head(nullptr), tail(nullptr) {}
        ~LRUCache() {
            std::cout << "destruct\n";
            while (siz > 0) {
                popBack();
            }
        }

        void setFile(FILE *_file) {
            file = _file;
        }

        void read(fpos_t diskPos, Val& val) {
            //DEBUG("read...")
            if (diskPos < 0) return; //invalid pos
            bool found = get(diskPos, val);
            if (!found) {
                fseek(file, diskPos, SEEK_SET);
                fread(reinterpret_cast<char *>(&val), sizeof(Val), 1, file);
                set(diskPos, val);
            } else {
                get(diskPos, val);
            }
        }

        void write(fpos_t diskPos, Val& val) {
            //DEBUG("write...")
            if (diskPos < 0) return; //invalid pos
            set(diskPos, val);
            /*fseek(file, diskPos, SEEK_SET);
            fwrite(reinterpret_cast<char *>(&val), sizeof(val), 1, file);*/
        }

        void writeParent(fpos_t diskPos, fpos_t parent) {
            if (table.find(diskPos) != table.end()) {
                table[diskPos]->val.parent = parent;
                return;
            }
            if (diskPos < 0) return; //invalid pos
            fseek(file, diskPos, SEEK_SET);
            fwrite(reinterpret_cast<char *>(&parent), sizeof(fpos_t), 1, file);
            /*Val tmpVal;
            fread(reinterpret_cast<char *>(&tmpVal), sizeof(Val), 1, file);
            tmpVal.parent = parent;
            fseek(file, diskPos, SEEK_SET);
            fwrite(reinterpret_cast<char *>(&tmpVal), sizeof(Val), 1, file);*/
        }

        void display() {
            Node *nowNode = head;
            std::cout << "* Cache *\n";
            std::cout << "size: " << siz << '\n';
            std::cout << "head: " << head << '\n';
            std::cout << "tail: " << tail << '\n';

            while (nowNode != nullptr) {
                std::cout << "[Node] key: " << nowNode->key << '\n';
                nowNode = nowNode->nxt;
            }
        }
    };
}

#endif //DS01_B_TREE_CACHE_HPP
