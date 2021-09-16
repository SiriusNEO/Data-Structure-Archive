#include "BinomialHeap.hpp"

using namespace Sirius;

int main() {
    DEBUG("start test")
    
    BinomialHeap<int> heap;
    for (int i = 1; i <= 3; ++i)
        heap.push(i);
    heap.display();
}