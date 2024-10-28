#pragma once

#include "component.hpp"
#include "level_definitions.hpp"
#include "manager.hpp"
#include "sprite.hpp"

// TODO: Refactor into POD only struct
struct SpriteInterpolated
{
  SpriteInterpolated() = default;
  SpriteInterpolated(const std::string &path)
    : sprite{ path }
  {
  }

  inline void update()
  {
    if (animation_speed != 0)
      sprite.animate(animation_speed);
  }

  inline void render();

  float x{ 0.0f };
  float y{ 0.0f };
  float previous_x{ std::numeric_limits<float>::quiet_NaN() };
  float previous_y{ std::numeric_limits<float>::quiet_NaN() };
  Sprite sprite;
  bool visible{ true };
  int animation_speed{ 1 };
};

struct SpriteRenderer final
{
  COMPONENT(SpriteRenderer);

  SpriteRenderer(const std::string &path)
    : sprite_interpolated{ path }
  {
  }

  inline void center()
  {
    sprite_interpolated.sprite.set_centered();
  }

  template<typename T>
  inline void set_position(T new_x, T new_y)
  {
    sprite_interpolated.x = static_cast<float>(new_x);
    sprite_interpolated.y = static_cast<float>(new_y);

    if (std::isnan(sprite_interpolated.previous_x))
    {
      sprite_interpolated.previous_x = sprite_interpolated.x;
      sprite_interpolated.previous_y = sprite_interpolated.y;
    }
  }

  inline void update()
  {
    sprite_interpolated.update();
  }

  void render();

  int depth{ 0 };
  SpriteInterpolated sprite_interpolated;
};

struct TileRenderer
{
  COMPONENT(TileRenderer);

  TileRenderer(const std::string &path, TilePosition pos, TileSize size, TileSourcePosition source)
    : x{ pos.x }
    , y{ pos.y }
    , source_x{ source.source_x }
    , source_y{ source.source_y }
    , w{ size.w }
    , h{ size.h }
    , sprite{ path }
  {
  }

  void render();
  static inline void render(Sprite &sprite, Rectangle source, Vector2 position);

  [[nodiscard]] inline auto get_width() const
  {
    return w;
  }
  [[nodiscard]] inline auto get_height() const
  {
    return h;
  }

  template<typename T>
  inline void set_position(T new_x, T new_y)
  {
    x = static_cast<int32_t>(new_x);
    y = static_cast<int32_t>(new_y);
  }

  inline void set_depth(int new_depth)
  {
    depth = new_depth;
  }

  [[nodiscard]] inline Rectangle source() const
  {
    return { static_cast<float>(source_x), static_cast<float>(source_y), static_cast<float>(w), static_cast<float>(h) };
  }

  [[nodiscard]] inline Rectangle dest() const
  {
    return { static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h) };
  }

  [[nodiscard]] inline const Texture2D &texture() const
  {
    return sprite.get_texture();
  }

  int depth{ 0 };

  int32_t x{ 0 };
  int32_t y{ 0 };

private:
  int32_t source_x{ 0 };
  int32_t source_y{ 0 };
  int32_t w{ 0 };
  int32_t h{ 0 };

  float previous_x{ 0.0f };
  float previous_y{ 0.0f };
  Sprite sprite;
  bool visible{ true };
};

EXTERN_COMPONENT_TEMPLATE(SpriteRenderer);
EXTERN_COMPONENT_TEMPLATE(TileRenderer);
