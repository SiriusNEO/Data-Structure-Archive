#include "BinomialHeap.hpp"

using namespace Sirius;

int main() {
    DEBUG("start test")
    
    BinomialHeap<int> heap;
    
    for (int i = 19; i >= 1; --i) heap.push(i);

    for (int i = 1; i <= 17; ++i) {
        std::cout << heap.top() << '\n';
        heap.pop();
    }

    heap.display();
}