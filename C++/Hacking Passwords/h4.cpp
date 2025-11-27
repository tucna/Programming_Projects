#include <iostream>
#include <cstring>

bool checkPassword(const char* input)
{
  // Password "REVERSE" XOR'd with 0x42
  const unsigned char encrypted[] = 
  {
    0x10, 0x07, 0x14, 0x07, 0x10, 0x11, 0x07, 0x00
  };

  const unsigned char key = 0x42;

  for (int i = 0; i < 7; i++)
  {
    if ((input[i] ^ key) != encrypted[i])
      return false;
  }

  return input[7] == '\0';
}

int main() {
  char input[64];
  std::cout << "Enter password: ";
  std::cin >> input;

  if (checkPassword(input))
    std::cout << "Correct!" << std::endl;
  else
    std::cout << "Wrong!" << std::endl;

  return 0;
}
