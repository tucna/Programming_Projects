// OS and Input
#include <windows.h>
#include <conio.h>

// Containers and Data
#include <vector>
#include <string>
#include <span>

// Utilities
#include <memory>    // std::unique_ptr
#include <algorithm> // std::ranges
#include <random>    // std::mt19937
#include <chrono>    // time units (ms)
#include <thread>    // std::this_thread

using namespace std::chrono_literals;

// =========================================================
// CONFIG
// =========================================================
constexpr int GRID_W = 56;
constexpr int GRID_H = 26;
constexpr int OFFSET_X = 4;
constexpr int OFFSET_Y = 2;

constexpr int SCREEN_WIDTH = (GRID_W * 2) + (OFFSET_X * 2);
constexpr int SCREEN_HEIGHT = GRID_H + (OFFSET_Y * 2) + 1;
constexpr int FPS = 20;

// Colors
constexpr WORD COL_WALL = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr WORD COL_P1 = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr WORD COL_P2 = FOREGROUND_RED | FOREGROUND_INTENSITY;
constexpr WORD COL_WIN = FOREGROUND_GREEN | FOREGROUND_INTENSITY;

// =========================================================
// TYPES & GLOBALS
// =========================================================
struct Pos { int x, y; };
enum class Dir { Up, Down, Left, Right };

struct ConsoleState
{
  HANDLE hOut;
  std::unique_ptr<CHAR_INFO[]> buffer;
  COORD bufferSize{ (SHORT)SCREEN_WIDTH, (SHORT)SCREEN_HEIGHT };
  COORD bufferCoord{ 0, 0 };
  SMALL_RECT writeRegion{ 0, 0, (SHORT)SCREEN_WIDTH - 1, (SHORT)SCREEN_HEIGHT - 1 };
};

std::mt19937 rng(std::random_device{}());

// =========================================================
// UTILITY
// =========================================================
void SetupConsole(ConsoleState& state)
{
  state.hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  state.buffer = std::make_unique<CHAR_INFO[]>(SCREEN_WIDTH * SCREEN_HEIGHT);

  SetConsoleScreenBufferSize(state.hOut, state.bufferSize);
  SetConsoleWindowInfo(state.hOut, TRUE, &state.writeRegion);

  HWND hwnd = GetConsoleWindow();
  SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

  CONSOLE_CURSOR_INFO ci;
  GetConsoleCursorInfo(state.hOut, &ci);
  ci.bVisible = FALSE;
  SetConsoleCursorInfo(state.hOut, &ci);
}

void DrawChar(ConsoleState& state, int x, int y, char c, WORD col)
{
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    return;

  auto& cell = state.buffer[y * SCREEN_WIDTH + x];
  cell.Char.AsciiChar = c;
  cell.Attributes = col;
}

void DrawGameChar(ConsoleState& state, int gx, int gy, char c, WORD col)
{
  int px = (gx * 2) + OFFSET_X;
  int py = gy + OFFSET_Y;
  DrawChar(state, px, py, c, col);
  DrawChar(state, px + 1, py, c, col);
}

Dir AI_choose(const std::vector<char>& grid, Pos p, Dir current)
{
  auto is_inside = [](int x, int y)
  { 
    return x >= 0 && x < GRID_W && y >= 0 && y < GRID_H;
  };

  auto valid = [&](Dir d)
  {
    int nx = p.x, ny = p.y;
    if (d == Dir::Up) ny--; else if (d == Dir::Down) ny++;
    else if (d == Dir::Left) nx--; else if (d == Dir::Right) nx++;
    return is_inside(nx, ny) && grid[ny * GRID_W + nx] == ' ';
  };

  if (valid(current) && (std::uniform_int_distribution<>(0, 100)(rng) > 10))
    return current;

  std::vector<Dir> dirs = { Dir::Up, Dir::Down, Dir::Left, Dir::Right };
  std::ranges::shuffle(dirs, rng);

  for (Dir d : dirs)
    if (valid(d))
      return d;

  return current;
}

// =========================================================
// MAIN
// =========================================================
int main()
{
  ConsoleState state;
  SetupConsole(state);

  while (true)
  {
    std::vector<char> grid(GRID_H * GRID_W, ' ');
    Pos p1 = { 5, GRID_H / 2 }, p2 = { GRID_W - 6, GRID_H / 2 };
    Dir d1 = Dir::Right, d2 = Dir::Left;
    bool running = true, p1Lost = false, p2Lost = false;

    while (running)
    {
      if (_kbhit())
      {
        char c = _getch();
        if (c == 'w' && d1 != Dir::Down) d1 = Dir::Up;
        else if (c == 's' && d1 != Dir::Up) d1 = Dir::Down;
        else if (c == 'a' && d1 != Dir::Right) d1 = Dir::Left;
        else if (c == 'd' && d1 != Dir::Left) d1 = Dir::Right;
        else if (c == 'q') return 0;
      }

      d2 = AI_choose(grid, p2, d2);
      grid[p1.y * GRID_W + p1.x] = '1';
      grid[p2.y * GRID_W + p2.x] = '2';

      auto move = [](Pos& p, Dir d)
      {
        if (d == Dir::Up) p.y--; else if (d == Dir::Down) p.y++;
        else if (d == Dir::Left) p.x--; else if (d == Dir::Right) p.x++;
      };

      move(p1, d1); move(p2, d2);

      auto collision = [&](Pos p)
      {
        return p.x < 0 || p.x >= GRID_W || p.y < 0 || p.y >= GRID_H || grid[p.y * GRID_W + p.x] != ' ';
      };

      if (collision(p1)) p1Lost = true;
      if (collision(p2)) p2Lost = true;
      if (p1Lost || p2Lost) running = false;

      // RENDER - Using std::span and ranges to clear buffer
      std::ranges::fill(std::span(state.buffer.get(), SCREEN_WIDTH * SCREEN_HEIGHT), CHAR_INFO{ (char)' ', 0 });

      // Draw Borders
      for (int x = -1; x <= GRID_W; x++)
      {
        DrawGameChar(state, x, -1, (char)219, COL_WALL);
        DrawGameChar(state, x, GRID_H, (char)219, COL_WALL);
      }

      for (int y = -1; y <= GRID_H; y++)
      {
        DrawGameChar(state, -1, y, (char)219, COL_WALL);
        DrawGameChar(state, GRID_W, y, (char)219, COL_WALL);
      }

      // Draw Trails
      for (int y = 0; y < GRID_H; ++y)
      {
        for (int x = 0; x < GRID_W; ++x)
        {
          char val = grid[y * GRID_W + x];
          if (val == '1') DrawGameChar(state, x, y, (char)176, COL_P1);
          if (val == '2') DrawGameChar(state, x, y, (char)176, COL_P2);
        }
      }

      // Draw Heads
      if (!p1Lost)
        DrawGameChar(state, p1.x, p1.y, (char)219, COL_P1 | FOREGROUND_GREEN);
      if (!p2Lost)
        DrawGameChar(state, p2.x, p2.y, (char)219, COL_P2 | FOREGROUND_GREEN);

      WriteConsoleOutputA(state.hOut, state.buffer.get(), state.bufferSize, state.bufferCoord, &state.writeRegion);
      std::this_thread::sleep_for(1000ms / FPS);
    }

    // WINNER LOGIC
    std::ranges::fill(std::span(state.buffer.get(), SCREEN_WIDTH * SCREEN_HEIGHT), CHAR_INFO{ (char)' ', 0 });

    std::string res;
    WORD resCol = COL_WIN;

    if (p1Lost && p2Lost)
    {
      res = " DRAW! BOTH CRASHED! ";
      resCol = COL_WALL;
    }
    else if (p1Lost)
    {
      res = " AI WINS! YOU CRASHED. ";
      resCol = COL_P2;
    }
    else
    {
      res = " YOU WIN! AI CRASHED. ";
      resCol = COL_P1;
    }

    int tx = (SCREEN_WIDTH / 2) - ((int)res.size() / 2);

    for (size_t i = 0; i < res.size(); i++)
      DrawChar(state, tx + (int)i, SCREEN_HEIGHT / 2, res[i], resCol);

    WriteConsoleOutputA(state.hOut, state.buffer.get(), state.bufferSize, state.bufferCoord, &state.writeRegion);
    std::this_thread::sleep_for(2500ms);
  }
}