#include "BinomialHeap.hpp"

using namespace Sirius;

#define CLOCKINIT() clock_t st = clock();
#define STANDINGBY() st = clock();
#define COMPLETE(_x) printf(_x": %.6lf\n", (clock()-st)/(double)CLOCKS_PER_SEC);

void pressure_test() {
    BinomialHeap<int> heap, heaps[50001];

    CLOCKINIT();
    
    STANDINGBY();
    for (int i = 1; i <= 1000000; ++i)
        heap.push(rand());
    COMPLETE("push");

    STANDINGBY();
    while (!heap.empty()) heap.pop();    
    COMPLETE("pop");

    for (int i = 1; i <= 15; ++i) {
        for (int j = 1; j <= 50000; ++j)
            heaps[j].push(rand());
    }

    STANDINGBY();
    for (int j = 1; j < 50000; ++j)
        heaps[j].merge(heaps[j+1]);
    COMPLETE("merge");
}

int main() {
    pressure_test();
}