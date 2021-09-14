//
// Created by SiriusNEO on 2021/9/14.
//

#include "SkipList.hpp"
#include <cassert>

#define CLOCKINIT() clock_t st = clock();
#define STANDINGBY() st = clock();
#define COMPLETE(_x) printf(_x": %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);

void basic_test() {
    Sirius::SkipList<int, int, 20> skipList;
    for (int i = 1; i <= 1000000; i++) {
        skipList.insert(i, i);
    }
    //skipList.display();
}

void pressure_test() {
    Sirius::SkipList<int, int, 20> skipList;
    CLOCKINIT()
    STANDINGBY()
    for (int i = 1; i <= 1000000; i++) {
        skipList.insert(i, i);
    }
    COMPLETE("insert")

    STANDINGBY()
    for (int i = 1; i <= 1000000; i++) {
        int x;
        bool found = skipList.find(i, x);
        assert(found);
    }
    COMPLETE("find")
}

int main() {
    pressure_test();
    return 0;
}

