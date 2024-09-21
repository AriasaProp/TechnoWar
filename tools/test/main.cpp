#include <iostream>

//test group
extern void stbi_rectpack_test ();

int main (int argc, char *argv[]) {
  std::cout << "Test Tools";
  try {
  	stbi_rectpack_test ();
  } catch (const char *err) {
  	std::cout << "\nError -> " << err << std::endl;
  	return EXIT_FAILURE;
  }
	std::cout << " Done!" << std::endl;
  return EXIT_SUCCESS;
}

