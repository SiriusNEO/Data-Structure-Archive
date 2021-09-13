//
// Created by SiriusNEO on 2021/9/13.
//

#include <map>
#include <cassert>
#include "AVLTree.hpp"

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
    Sirius::AVLTree<int, int> avl;
    for (int i = 5; i >= 1; --i)
        avl.insert(i, i);

    DEBUG("insert finish")

    for (int i = 1; i <= 6; ++i) {
        int x;
        bool found = avl.modify(i, -i);
        if (found) {
            avl.find(i, x);
            std::cout << x << '\n';
        } else {
            std::cout << "404\n";
        }
    }

    DEBUG("modify finish")

    avl.del(3);

    avl.display();
}

void order_test() {
    srand(time(0));
    Sirius::AVLTree<int, int> avlTree;

    std::map<int, int> std_map;

    const int TOTAL = 1000000, DELETE = 1000000;

    CLOCKINIT()

    std::cout << "insert test\n";
    STANDINGBY()
    for (int i = 1; i <= TOTAL; i++) {
        std_map[i] = i;
        avlTree.insert(i, i);
        //std::cout << i << '\n';
    }

    COMPLETE("insert")

    std::cout << "find test\n";
    STANDINGBY()
    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        int result = -1;
        bool found = avlTree.find(it->first, result);
        assert(found);
        //std::cout << "result: " << result << '\n';
    }
    COMPLETE("find")

    std::cout << "del test\n";
    int cnt = 0;
    STANDINGBY()
    for (auto it = std_map.begin(); cnt < DELETE; it++, cnt++) {
        avlTree.del(it->first);
        //std::cout << cnt << '\n';
    }
    COMPLETE("del")
    avlTree.display();
}

void string_test() {
    srand(time(0));
    Sirius::AVLTree<std::string, int> avlTree;

    std::map<std::string, int> std_map;

    const int TOTAL = 1000000, DELETE = 1000000;

    CLOCKINIT()

    std::cout << "insert test\n";
    STANDINGBY()
    for (int i = 1; i <= TOTAL; i++) {
        std::string ins_str = randString(32);
        std_map[ins_str] = i;
        avlTree.insert(ins_str, i);
        //std::cout << i << '\n';
    }

    COMPLETE("insert")

    std::cout << "find test\n";
    STANDINGBY()
    for (auto it = std_map.begin(); it != std_map.end(); it++) {
        int result = -1;
        bool found = avlTree.find(it->first, result);
        assert(found);
        //std::cout << "result: " << result << '\n';
    }
    COMPLETE("find")

    std::cout << "del test\n";
    int cnt = 0;
    STANDINGBY()
    for (auto it = std_map.begin(); cnt < DELETE; it++, cnt++) {
        avlTree.del(it->first);
        //std::cout << cnt << '\n';
    }
    COMPLETE("del")
    avlTree.display();
}

int main() {
    string_test();
}

