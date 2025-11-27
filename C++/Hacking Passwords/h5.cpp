#include <iostream>
#include <windows.h>
#include <string>

unsigned int NotXXH32(const void* input, size_t length, unsigned int seed) 
{
  const unsigned char* p = (const unsigned char*)input;
  const unsigned char* const end = p + length;
  unsigned int h = seed + 0x9E3779B1 + (unsigned int)length;

  while (p + 4 <= end)
  {
    h += *(unsigned int*)p * 0x165667B1;
    h = (h << 11) | (h >> 21);
    h *= 0x9E3779B1;
    p += 4;
  }

  while (p < end)
  {
    h += *p++ * 5;
    h = (h << 7) | (h >> 25);
    h *= 0x9E3779B1;
  }

  h ^= h >> 15;
  h *= 0x85EBCA6B;
  h ^= h >> 13;
  h *= 0xC2B2AE35;
  h ^= h >> 16;

  return h;
}

bool checkPassword(const char* input)
{
  if (IsDebuggerPresent()) 
  {
    std::cout << "Debugger not allowed." << std::endl;
    return false;
  }

  // Pre-computed hash of the real password "NINJA" with seed 0x1337
  // You generate this once and paste the number
  const unsigned int correct_hash = 0x5caf0295;

  unsigned int user_hash = NotXXH32(input, strlen(input), 0x1337);

  return user_hash == correct_hash;
}

int main()
{
  std::string input;
  std::cout << "Enter password: ";
  std::getline(std::cin, input);

  if (checkPassword(input.c_str()))
    std::cout << "Access granted!" << std::endl;
  else
    std::cout << "Access denied!" << std::endl;

  return 0;
}
