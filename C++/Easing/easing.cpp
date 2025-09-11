#define T_PGE_APPLICATION
#include "engine/tPixelGameEngine.h"

using EasingFunc = std::function<float(float, float, float, float)>;

namespace easing
{
  float linear(float t, float b, float c, float d)
  {
    return c * t / d + b;
  }

  float easeInQuad(float t, float b, float c, float d)
  {
    t /= d;
    return c * t * t + b;
  }

  float easeOutQuad(float t, float b, float c, float d)
  {
    t /= d;
    return -c * t * (t - 2.0f) + b;
  }

  float easeInOutQuad(float t, float b, float c, float d)
  {
    t /= d / 2.0f;
    if (t < 1.0f) return c / 2.0f * t * t + b;
    t--;
    return -c / 2.0f * (t * (t - 2.0f) - 1.0f) + b;
  }

  float easeOutBounce(float t, float b, float c, float d)
  {
    t /= d;
    if (t < (1.0f / 2.75f))
    {
      return c * (7.5625f * t * t) + b;
    }
    else if (t < (2.0f / 2.75f))
    {
      t -= (1.5f / 2.75f);
      return c * (7.5625f * t * t + 0.75f) + b;
    }
    else if (t < (2.5f / 2.75f))
    {
      t -= (2.25f / 2.75f);
      return c * (7.5625f * t * t + 0.9375f) + b;
    }
    else
    {
      t -= (2.625f / 2.75f);
      return c * (7.5625f * t * t + 0.984375f) + b;
    }
  }

  float easeOutElastic(float t, float b, float c, float d)
  {
    if (t == 0) return b;
    t /= d;
    if (t == 1) return b + c;
    float p = d * 0.3f;
    float a = c;
    float s = p / 4.0f;
    return (a * pow(2.0f, -10.0f * t) * sin((t * d - s) * (2.0f * 3.14159f) / p) + c + b);
  }

  struct Animation
  {
    std::string name;
    EasingFunc func;
    tDX::Pixel color;
    std::string formula;
    float yScale = 1.0f;
  };
}

class EasingFunctions : public tDX::PixelGameEngine
{
public:
  static const int SCREEN_WIDTH = 900;
  static const int SCREEN_HEIGHT = 920;

public:
  EasingFunctions()
  {
    sAppName = "Easing Functions";

    _animations =
    {
        { "Linear", easing::linear, tDX::Pixel(52, 152, 219), "f(x) = x" },
        { "Ease-In", easing::easeInQuad, tDX::Pixel(46, 204, 113), "f(x) = x^2" },
        { "Ease-Out", easing::easeOutQuad, tDX::Pixel(231, 76, 60), "f(x) = 1 - (1-x)^2" },
        { "Elastic", easing::easeOutElastic, tDX::Pixel(26, 188, 156), "Elastic Decay", 1.4f },
        { "Ease-In-Out", easing::easeInOutQuad, tDX::Pixel(241, 196, 15), "S-Curve (Conditional)" },
        { "Bounce", easing::easeOutBounce, tDX::Pixel(155, 89, 182), "Bouncing Decay" }
    };
  }

  bool OnUserCreate() override
  {
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(tDX::Pixel(40, 44, 52));

    _animationTime += fElapsedTime;

    if (_animationTime >= _duration)
    {
      _animationTime -= _duration;
    }

    for (size_t i = 0; i < _animations.size(); ++i)
    {
      const auto& anim = _animations[i];
      int32_t nSectionY = _sectionHeight * i;

      // --- Draw Animation ---
      float currentX = anim.func(_animationTime, (float)_startX, (float)_distance, _duration);
      int32_t nAnimY = nSectionY + 80;

      DrawLine(_startX, nAnimY, _startX + _distance, nAnimY, tDX::Pixel(255, 255, 255, 40));
      FillCircle((int32_t)currentX, nAnimY, _radius, anim.color);
      DrawCircle((int32_t)currentX, nAnimY, _radius, tDX::Pixel(0, 0, 0, 80));

      // --- Draw Labels & Graph ---
      int32_t nLabelY = nSectionY + 25;
      DrawString(_startX, nLabelY, anim.name, tDX::WHITE, 2);

      int32_t nGraphHeight = 60;
      int32_t nGraphX = _endX - _graphWidth;
      int32_t nGraphY = nLabelY + 20;

      DrawString(nGraphX - 10, nLabelY, anim.formula, tDX::GREY);

      plotFunction(anim, nGraphX, nGraphY, _graphWidth, nGraphHeight);
    }

    return true;
  }

private:
  void plotFunction(const easing::Animation& anim, int32_t x, int32_t y, int32_t w, int32_t h)
  {
    // Draw axes
    DrawLine(x, y, x, y + h, tDX::Pixel(255, 255, 255, 50));
    DrawLine(x, y + h, x + w, y + h, tDX::Pixel(255, 255, 255, 50));

    // Draw function curve
    tDX::vi2d p1 = { x, y + h };

    for (float t_norm = 0.01f; t_norm <= 1.0f; t_norm += 0.01f)
    {
      float val_norm = anim.func(t_norm, 0.0f, 1.0f, 1.0f);
      int32_t plotX = x + (int32_t)(t_norm * w);
      int32_t plotY = (y + h) - (int32_t)((val_norm / anim.yScale) * h);
      tDX::vi2d p2 = { plotX, plotY };
      DrawLine(p1, p2, anim.color);
      p1 = p2;
    }
  }

private:
  const float _duration = 4.0f; // 4 seconds

  std::vector<easing::Animation> _animations;
  float _animationTime = 0.0f;

  int32_t _sectionHeight = 150;
  int32_t _radius = 20;
  int32_t _startX = 60;
  int32_t _graphWidth = 120;
  int32_t _graphPadding = 40;
  int32_t _endX = SCREEN_WIDTH - 60;
  int32_t _distance = _endX - _startX - _graphWidth - _graphPadding;
};


int main()
{
  EasingFunctions ef;
  if (ef.Construct(ef.SCREEN_WIDTH, ef.SCREEN_HEIGHT, 1, 1))
    ef.Start();

  return 0;
}
