#include <iostream>

extern bool own_rectpack_test ();

int main () {
  bool passed = true;
  std::cout << "Start Test - Tools module" << std::endl;
  passed &= own_rectpack_test ();
  std::cout << "End Test - Tools module" << std::endl;
  return 0;
}