//
// Created by SiriusNEO on 2021/9/13.
//

#include <map>
#include <cassert>
#include "RedBlackTree.hpp"

#define CLOCKINIT() clock_t st = clock();
#define STANDINGBY() st = clock();
#define COMPLETE(_x) printf(_x": %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);

inline int randInt(int l, int r) {
    return rand()%(r-l+1)+l;
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

void basic_test() {
    Sirius::RedBlackTree<int, int> rbtree;
    for (int i = 1; i <= 10; i++) rbtree.insert(std::make_pair(i, i));
    rbtree.display();
}

void string_test() {
    srand(time(0));
    Sirius::RedBlackTree<std::string, int> rbtree;

    std::map<std::string, int> std_map;

    const int TOTAL = 1000000, DELETE = 1000000;

    CLOCKINIT()

    std::cout << "insert test\n";
    STANDINGBY()
    for (int i = 1; i <= TOTAL; i++) {
        std::string ins_str = randString(32);
        std_map[ins_str] = i;
        rbtree.insert(std::make_pair(ins_str, i));
        //std::cout << i << '\n';
    }

    COMPLETE("insert")

    std::cout << "find test\n";
    STANDINGBY()
    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        int result = -1;
        bool found = rbtree.find(it->first, result);
        assert(found);
        //std::cout << "result: " << result << '\n';
    }
    COMPLETE("find")

    std::cout << "del test\n";
    int cnt = 0;
    STANDINGBY()
    for (auto it = std_map.begin(); cnt < DELETE; it++, cnt++) {
        rbtree.del(it->first);
        //std::cout << cnt << '\n';
    }
    COMPLETE("del")
    std::cout << rbtree.size() << '\n';
}

int main() {
    string_test();
}

