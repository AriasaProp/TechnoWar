#include <iostream>

extern bool stbi_rectpack_test ();

int main () {
  bool passed = true;
  std::cout << "Start Test - Tools module" << std::endl;
  passed &= stbi_rectpack_test ();

  std::cout << "End Test - Tools module" << std::endl;
  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}