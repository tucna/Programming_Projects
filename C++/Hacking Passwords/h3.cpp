#include <iostream>
#include <cstring>
#include <windows.h>

bool checkPassword(const char* input)
{
  // Anti-debugging check
  if (IsDebuggerPresent())
  {
    std::cout << "Debugger detected!" << std::endl;
    return false;
  }

  char password[6];
  password[0] = 'N';
  password[1] = 'I';
  password[2] = 'N';
  password[3] = 'J';
  password[4] = 'A';
  password[5] = '\0';

  return strcmp(input, password) == 0;
}

int main() {
  char input[64];
  std::cout << "Enter password: ";
  std::cin >> input;

  if (checkPassword(input))
    std::cout << "Access granted!" << std::endl;
  else
    std::cout << "Access denied!" << std::endl;

  return 0;
}
