#include <iostream>
#include "persistent_set.h"

using std::cin;
using std::cout;
using std::endl;

int main() {
    persistent_set<int> ps;
    ps.insert(1);
    ps.insert(2);
    ps.insert(3);
    ps.erase(ps.begin());
    cout << *(ps.begin()) << endl;
    return 0;
}