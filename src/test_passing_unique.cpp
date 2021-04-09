#include <iostream>
#include <memory>

using namespace std;

void f1(int * a) {
    cout << *a << endl;
}

int main() {
    auto var = make_unique<int>(5);
	f1(var.release());
	cout << *var << endl;
    return 0;
}
