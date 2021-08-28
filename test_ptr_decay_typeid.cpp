#include <iostream>
using namespace std;


int main() {
const char t1[2] = "\n";
const char* t2 = "\n";

cout << typeid(t1).name() << endl;
cout << typeid(t2).name() << endl;
cout << typeid(std::decay<decltype(t1)>::type).name() << endl;
return 0;
}
