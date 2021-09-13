//
// Created by SiriusNEO on 2021/9/13.
//

#include "AVLTree.hpp"

int main() {
    Sirius::AVLTree<int, int> avl;
    for (int i = 5; i >= 1; --i)
        avl.insert(i, i);

    DEBUG("insert finish")

    for (int i = 1; i <= 6; ++i) {
        int x;
        bool found = avl.find(i, x);
        if (found) {
            std::cout << x << '\n';
        } else {
            std::cout << "404\n";
        }
    }
    avl.display();
}

