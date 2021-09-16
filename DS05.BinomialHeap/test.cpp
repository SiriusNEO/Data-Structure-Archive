#include <iostream>
#include <memory>

struct A {
    int b;
};

using namespace std;

int main() {
    shared_ptr<int> q;
    shared_ptr<int> p = make_shared<int>(2);
    q = make_shared<int>(3);
    cout << (q == nullptr) << '\n';
}