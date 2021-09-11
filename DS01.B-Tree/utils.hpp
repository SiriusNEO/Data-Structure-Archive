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
#include <ctime>

#define INS(_x) btree.insert(_x, _x);

#define CLOCKINIT() clock_t st = clock();
#define STANDINGBY() st = clock();
#define COMPLETE(_x) printf(_x": %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);

inline int randInt(int l, int r) {
    return rand()%(r-l+1)+l;
}

void cache_write_test() {
    FILE *file = fopen("test.db", "rb+");
    Sirius::LRUCache<int, 5> cache;
    cache.setFile(file);

    for (int i = 0; i < 100000; i++) {
        cache.write(i*4, i);
        int result;
        cache.read(i*4, result);
        std::cout << result << '\n';
        //cache.write(i*4, -i);
    }

    cache.display();
}

void cache_read_test() {
    FILE *file = fopen("test.db", "rb+");
    Sirius::LRUCache<int, 5> cache;
    cache.setFile(file);

    for (int i = 0; i < 100000; i++) {
        int x;
        cache.read(i*4, x);
        std::cout << x << '\n';
    }
}

inline std::string randString(int size = randInt(1, 1000)) {
    std::string ret;
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
    Sirius::BTree<int, int, 512> btree("data.db");
    for (int i = 1; i <= 100000; i++) INS(i)
    //btree.display();
}

void insert_test2() {
    Sirius::BTree<int, int, 5> btree("data.db");
    for (int i = 10; i >= 1; i--) {
        INS(i)
    }
    btree.display();
}

void del_test1() {
    Sirius::BTree<int, int, 5> btree("data.db");
    for (int i = 1; i <= 30000; i++) INS(i)
    for (int i = 1; i <= 3000; i++) {
        for (int j = i*10; j >= (i-1)*10+1; --j) {
            btree.del(j);
        }
    }
    btree.display();

    /*for (int i = 1; i <= 6; ++i) {
        btree.del((i-1)*5 + 1);
        btree.display();
        btree.del((i-1)*5 + 5);
        btree.display();
        btree.del((i-1)*5 + 4);
        btree.display();
        btree.del((i-1)*5 + 3);
        btree.display();
        btree.del((i-1)*5 + 2);
        btree.display();
    }*/
}

void del_test2() {
    Sirius::BTree<std::string, int, 5> btree("data.db");
    std::map<int, bool> keyMap;
    for (int i = 30000; i <= 60000; i++) {
        btree.insert(std::to_string(i), i);
        assert(!keyMap[std::hash<std::string>{}(std::to_string(i)) % 1073741827]);
        keyMap[std::hash<std::string>{}(std::to_string(i)) % 1073741827] = 1;
        int x;
        bool founded = btree.find(std::to_string(i), x);
        if (!founded) {
            std::cout << "404\n";
            return;
        }
    }
    for (int i = 30000; i <= 60000; i++) {
        std::cout << i << '\n';
        bool del = btree.del(std::to_string(i));
        /*if (!del) {
            std::cout << std::hash<std::string>{}(std::to_string(i)) % 1073741827 << '\n';
            return;
        }*/
    }
    btree.display();
}

void string_test() {
    srand(time(0));
    Sirius::BTree<std::string, int, 512> btree("data.db");
    std::map<std::string, int> std_map;

    const int TOTAL = 100000, DELETE = 100000;

    CLOCKINIT()

    std::cout << "insert test\n";
    STANDINGBY()
    for (int i = 1; i <= TOTAL; i++) {
        std::string ins_str = randString(32);
        std_map[ins_str] = i;
        btree.insert(ins_str, i);
        //std::cout << i << '\n';
    }
    COMPLETE("insert")

    std::cout << "find test\n";
    STANDINGBY()
    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        int result = -1;
        bool found = btree.find(it->first, result);
        assert(found);
        //std::cout << "result: " << result << '\n';
    }
    COMPLETE("find")

    std::cout << "del test\n";
    int cnt = 0;
    STANDINGBY()
    for (auto it = std_map.begin(); cnt < DELETE; it++, cnt++) {
        btree.del(it->first);
        std::cout << cnt << '\n';
    }
    COMPLETE("del")
    btree.display();
}

#endif //DS01_B_TREE_UTILS_HPP
