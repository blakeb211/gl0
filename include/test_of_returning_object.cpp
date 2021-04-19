#include <iostream>
#include <memory>
#include <string>

using namespace std;

struct object {
    object() {}
    std::string name;
};

unique_ptr<object> do_stuff() {
    auto objectPtr = std::make_unique<object>();

    string entityName = "testName";
    objectPtr->name = entityName;

    return (std::move(objectPtr));
}

int main() {
    auto result = do_stuff();
    cout << "Name: " << result->name << endl;
    return 0;
}
