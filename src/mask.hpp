#pragma once

#include <raylib.h>

struct Mask
{
  int origin_x{ 0 };
  int origin_y{ 0 };
  int width{ 0 };
  int height{ 0 };

  [[nodiscard]] static constexpr inline Mask center_rect(int w, int h)
  {
    return Mask{ w / 2, h / 2, w, h };
  }

  [[nodiscard]] inline Rectangle rect(int x, int y) const
  {
    return { static_cast<float>(x - origin_x),
             static_cast<float>(y - origin_y),
             static_cast<float>(width),
             static_cast<float>(height) };
  }

  [[nodiscard]] inline Vector2 top_left(int x, int y) const
  {
    return { static_cast<float>(x - origin_x), static_cast<float>(y - origin_y) };
  }

  [[nodiscard]] inline Vector2 top_right(int x, int y) const
  {
    return { static_cast<float>(x - origin_x + width), static_cast<float>(y - origin_y) };
  }

  [[nodiscard]] inline Vector2 bottom_left(int x, int y) const
  {
    return { static_cast<float>(x - origin_x), static_cast<float>(y - origin_y + height) };
  }

  [[nodiscard]] inline Vector2 bottom_right(int x, int y) const
  {
    return { static_cast<float>(x - origin_x + width), static_cast<float>(y - origin_y + height) };
  }

  [[nodiscard]] inline float left(int x) const
  {
    return static_cast<float>(x - origin_x);
  }

  [[nodiscard]] inline float right(int x) const
  {
    return static_cast<float>(x - origin_x + width);
  }

  [[nodiscard]] inline float top(int y) const
  {
    return static_cast<float>(y - origin_y);
  }

  [[nodiscard]] inline float bottom(int y) const
  {
    return static_cast<float>(y - origin_y + height);
  }

  void draw(int x, int y) const
  {
    const auto &rec = rect(x, y);
    DrawRectangleLinesEx(rec, 1, RBLACK);
  }
};
