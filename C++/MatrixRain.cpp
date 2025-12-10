#include <windows.h>
#include <iostream>
#include <vector>

const int SCREEN_WIDTH = 120;
const int SCREEN_HEIGHT = 40;

HANDLE hConsole;
CHAR_INFO* consoleBuffer;
COORD bufferSize;
COORD bufferCoord = { 0, 0 };
SMALL_RECT writeRegion;

struct Droplet
{
  float y;
  float speed;
  int length;
};

std::vector<Droplet> drops;

char GetRandomChar()
{
  int r = rand() % 3;
  if (r == 0) return '0' + rand() % 10;
  if (r == 1) return 'A' + rand() % 26;
  return 33 + rand() % 15;
}

void SetupConsole()
{
  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

  SMALL_RECT windowSize = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };
  SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

  bufferSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
  SetConsoleScreenBufferSize(hConsole, bufferSize);

  writeRegion = { 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1 };

  consoleBuffer = new CHAR_INFO[SCREEN_WIDTH * SCREEN_HEIGHT];

  CONSOLE_CURSOR_INFO ci;
  GetConsoleCursorInfo(hConsole, &ci);
  ci.bVisible = FALSE;
  SetConsoleCursorInfo(hConsole, &ci);

  drops.resize(SCREEN_WIDTH);
  for (int i = 0; i < SCREEN_WIDTH; i++)
  {
    drops[i].y = -(rand() % SCREEN_HEIGHT);
    drops[i].speed = 0.5f + ((rand() % 10) / 10.0f);
    drops[i].length = 5 + rand() % 20;
  }
}

void ClearAndDraw()
{
  // Clear the entire console manually
  for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
  {
    consoleBuffer[i].Char.AsciiChar = ' ';
    consoleBuffer[i].Attributes = 0;
  }

  for (int x = 0; x < SCREEN_WIDTH; x++)
  {
    Droplet& d = drops[x];

    // Move
    d.y += d.speed;
    if (d.y >= SCREEN_HEIGHT)
      d.y = -(rand() % 10);

    // Draw
    int headY = (int)d.y;

    for (int i = 0; i < d.length; i++)
    {
      int yPos = headY - i;
      if (yPos < 0 || yPos >= SCREEN_HEIGHT)
        continue;

      int idx = yPos * SCREEN_WIDTH + x;

      if (rand() % 20 == 0)
        consoleBuffer[idx].Char.AsciiChar = GetRandomChar();
      else if (consoleBuffer[idx].Char.AsciiChar == ' ')
        consoleBuffer[idx].Char.AsciiChar = GetRandomChar();

      if (i == 0)
        consoleBuffer[idx].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
      else if (i < 3)
        consoleBuffer[idx].Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
      else
        consoleBuffer[idx].Attributes = FOREGROUND_GREEN;
    }
  }

  WriteConsoleOutputA(
    hConsole,
    consoleBuffer,
    bufferSize,
    bufferCoord,
    &writeRegion
  );
}

int main()
{
  SetupConsole();

  while (true)
  {
    ClearAndDraw();
    Sleep(33); // ~30 FPS
  }

  return 0;
}
