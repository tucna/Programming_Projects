#include <iostream>
#include <cstring>

bool checkPassword(const char* input)
{
  const char* password = "C0D3BREAK";
  int len = strlen(password);

  for (int i = 0; i < len; i++)
  {
    if (input[i] != password[i])
      return false;
  }

  return input[len] == '\0';
}

int main()
{
  char input[64];
  std::cout << "Enter password: ";
  std::cin >> input;

  if (checkPassword(input))
    std::cout << "Success!" << std::endl;
  else
    std::cout << "Failed!" << std::endl;

  return 0;
}
