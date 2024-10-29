#include <iostream>

extern void qoi_test();

int main () {
  std::cout << "Test Core " << std::endl;
  if (!qoi_test()) {
  	return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}