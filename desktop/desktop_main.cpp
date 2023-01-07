#include <iostream>

using namespace std;
int main () {
    int a=5;     // initial value = 5
    int b(2);    // initial value = 2
    int result;  // initial value undetermined

    a = a + 4;
    result = a - b;
    cout << result;

    return 0;
}