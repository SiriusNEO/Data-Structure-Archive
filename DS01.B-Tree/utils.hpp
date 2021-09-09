//
// Created by SiriusNEO on 2021/9/9.
//

#ifndef DS01_B_TREE_UTILS_HPP
#define DS01_B_TREE_UTILS_HPP

#include "BTree.hpp"
#include <iostream>
#include <cstdlib>
#include <string>
#include <map>

#define INS(_x) btree.insert(_x, _x);

inline int randInt(int l, int r) {
    return rand()%(r-l+1)+l;
}

inline std::string randString(int size = randInt(1, 1000)) {
    std::string  ret;
    while (size--) {
        int typ = randInt(1, 64);
        if (typ >= 1 && typ <= 10) ret += (char)randInt('0', '9');
        else if (typ >= 11 && typ <= 36) ret += (char)randInt('a', 'z');
        else if (typ >= 37 && typ <= 62) ret += (char)randInt('A', 'Z');
        else if (typ == 63) ret += '@';
        else if (typ == 64) ret += '_';
    }
    return ret;
}

void basic_test1() {
    Sirius::BTree<int, int, 6> btree("data.db");
    for (int i = 1; i <= 13; ++i) INS(i)

    btree.display();

    int res;
    for (int i = 0; i <= 14; ++i) {
        bool founded = btree.modify(i, -i);
        if (founded) {
            printf("modified: %d\n", i);
        }
        else {
            printf("404\n");
        }
    }

    for (int i = 0; i <= 14; ++i) {
        bool founded = btree.find(i, res);
        if (founded) {
            printf("result: %d\n", res);
        }
        else {
            printf("404\n");
        }
    }
}

void insert_test() {
    Sirius::BTree<int, int, 6> btree("data.db");
    for (int i = 1; i <= 30; i++) INS(i)
    btree.display();
}

void del_test1() {
    Sirius::BTree<int, int, 6> btree("data.db");
    for (int i = 1; i <= 10; i++) INS(i)
    for (int i = 1; i <= 10; i++) btree.del(i);
    for (int i = 1; i <= 10; i++) INS(i)
    for (int i = 1; i <= 10; i++) btree.del(i);
    for (int i = 1; i <= 10; i++) INS(i)
    btree.display();
}

void del_test2() {
    Sirius::BTree<std::string, int, 5> btree("data.db");
    for (int i = 1; i <= 30; i++) {
        btree.insert(std::to_string(i), i);
    }
    btree.display();
    for (int i = 1; i <= 10; i++) {
        std::cout << "key: " << std::hash<std::string>{}(std::to_string(i)) % ((1<<30) - 1) << '\n';
        std::cout << btree.del(std::to_string(i)) << '\n';
        if (i == 5) btree.display();
    }
    btree.display();
}

void string_test() {
    srand(time(0));
    Sirius::BTree<std::string, int, 1000> btree("data.db");
    std::map<std::string, int> std_map;

    const int TOTAL = 100000, DELETE = 10000;

    for (int i = 1; i <= TOTAL; i++) {
        std::string ins_str = randString(32);
        std_map[ins_str] = i;
        std::cout << i << ' ' << btree.insert(ins_str, i) << '\n';
    }

    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        int result = -1;
        std::cout << btree.find(it->first, result) << '\n';
        std::cout << "result: " << result << '\n';
    }

    int cnt = 0;
    for (auto it = std_map.begin(); cnt <= DELETE; it++, cnt++) {
        btree.del(it->first);
    }
    std::cout << "del" << '\n';
    cnt = 0;
    for (auto it = std_map.begin(); cnt <= DELETE + 5; it++, cnt++) {
        int result = -1;
        std::cout << btree.find(it->first, result) << '\n';
        std::cout << "result: " << result << '\n';
    }
}

#endif //DS01_B_TREE_UTILS_HPP
