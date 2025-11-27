#include <iostream>
#include <cstring>

int main()
{
  const char* password = "SECRET123";
  char input[64];

  std::cout << "Enter password: ";
  std::cin >> input;

  if (strcmp(input, password) == 0)
  {
    std::cout << "Access Granted!" << std::endl;
    return 0;
  }

  std::cout << "Access Denied!" << std::endl;
  return 1;
}
