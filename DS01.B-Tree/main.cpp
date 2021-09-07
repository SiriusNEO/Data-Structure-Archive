#include "BTree.hpp"

#define INS(_x) btree.insert(_x, _x);

Sirius::BTree<int, int, 6> btree("data.db");

void basic_test1() {
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
    for (int i = 1; i <= 30; i++) INS(i)
    btree.display();
}

void del_test() {
    for (int i = 1; i <= 10; i++) INS(i)
    for (int i = 1; i <= 10; i++) btree.del(i);
    for (int i = 1; i <= 10; i++) INS(i)
    btree.display();
}

int main() {
    del_test();
	return 0;
}
