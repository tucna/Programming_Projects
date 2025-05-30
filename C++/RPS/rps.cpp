#define T_PGE_APPLICATION
#include "engine/tPixelGameEngine.h"

#include <random>

class RockPaperScissors : public tDX::PixelGameEngine
{
public:

  static const int N = 200;
  static const int SCREEN_WIDTH = 1440;
  static const int SCREEN_HEIGHT = 804;

  enum class Sign
  {
    Rock, Paper, Scissors
  };

  struct Item
  {
    Sign sign;
    double pos_x, pos_y;
    double vec_x, vec_y;
  };

  Item papers_c = { Sign::Paper, 100, 600, 0.0, 0.0 };
  Item scissors_c = { Sign::Scissors, 600, 100, 0.0, 0.0 };
  Item rocks_c = { Sign::Rock, 1000, 600, 0.0, 0.0 };

  Item items[N * 3];

  tDX::Sprite sc;
  tDX::Sprite pa;
  tDX::Sprite ro;

  RockPaperScissors()
  {
    sAppName = "Rock Paper Scissors";
  }

  bool OnUserCreate() override
  {
    // Called once at the beginning, so create things here

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> shift(-50, 50);
    std::uniform_real_distribution<> move(-1.0, 1.0);

    for (int i = 0; i < N * 3; i++)
    {
      Item item = { Sign::Rock, 0, 0, 0, 0 };

      if (i < N)
        item = rocks_c;
      else if (i < N * 2)
        item = papers_c;
      else
        item = scissors_c;

      items[i] = { item.sign, item.pos_x + shift(gen), item.pos_y + shift(gen), item.vec_x + move(gen), item.vec_y + move(gen) };
    }

    sc.LoadFromFile("s.png");
    pa.LoadFromFile("p.png");
    ro.LoadFromFile("r.png");

    Sleep(1000);

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::BLACK);
    SetPixelMode(tDX::Pixel::Mode::ALPHA);

    for (int i = 0; i < N * 3; i++)
    {
      items[i].pos_x += items[i].vec_x;
      items[i].pos_y += items[i].vec_y;

      items[i].vec_x *= (items[i].pos_x) < 10.0 || (items[i].pos_x) >= SCREEN_WIDTH - 30 ? -1.0 : 1.0;
      items[i].vec_y *= (items[i].pos_y) < 10.0 || (items[i].pos_y) >= SCREEN_HEIGHT - 30 ? -1.0 : 1.0;
    }

    for (int i = 0; i < N * 3; i++)
    {
      switch (items[i].sign)
      {
      case Sign::Paper: DrawSprite(items[i].pos_x, items[i].pos_y, &pa); break;
      case Sign::Rock: DrawSprite(items[i].pos_x, items[i].pos_y, &ro); break;
      case Sign::Scissors: DrawSprite(items[i].pos_x, items[i].pos_y, &sc); break;
      };
    }

    int x1, y1;
    int x2, y2;
    Sign s1, s2;

    for (int i = 0; i < N * 3; i++)
    {
      x1 = items[i].pos_x + 10;
      y1 = items[i].pos_y + 10;
      s1 = items[i].sign;

      for (int o = 0; o < N * 3; o++)
      {
        x2 = items[o].pos_x + 10;
        y2 = items[o].pos_y + 10;
        s2 = items[o].sign;

        if (i == o || s1 == s2) continue;

        if (abs(x1 - x2) < 10 && abs(y1 - y2) < 10)
        {
          switch (s1)
          {
          case Sign::Rock:
            if (s2 == Sign::Scissors) items[o].sign = Sign::Rock;
            else if (s2 == Sign::Paper) items[i].sign = Sign::Paper;
            break;
          case Sign::Paper:
            if (s2 == Sign::Rock) items[o].sign = Sign::Paper;
            else if (s2 == Sign::Scissors) items[i].sign = Sign::Scissors;
            break;
          case Sign::Scissors:
            if (s2 == Sign::Paper) items[o].sign = Sign::Scissors;
            else if (s2 == Sign::Rock) items[i].sign = Sign::Rock;
            break;
          }
        }

      }
    }

    return true;
  }
};


int main()
{
  RockPaperScissors rps;
  if (rps.Construct(rps.SCREEN_WIDTH, rps.SCREEN_HEIGHT, 1, 1))
    rps.Start();

  return 0;
}
