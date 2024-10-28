#pragma once

#include <set>
#include <string>

#include <raylib.h>
#include <raymath.h>

#include "component.hpp"
#include "manager.hpp"

#include "mask.hpp"

struct Physics
{
  COMPONENT(Physics);

  void update();

#if defined(DEBUG)
  void render()
  {
    if (IsKeyDown(KEY_F1))
      mask.draw(x, y);
  }
#endif

  int x_previous{ 0 };
  int y_previous{ 0 };
  int x{ 0 };
  int y{ 0 };
  Vector2 v{ 0.0f, 0.0f };
  Vector2 v_previous{ 0.0f, 0.0f };
  Vector2 v_rem{ 0.0f, 0.0f };
  float gravity{ 0.0f };

  bool collidable : 1 { true };
  bool solid : 1 { false };
  bool movable : 1 { true };
  bool oneway : 1 { false };
  bool do_update : 1 { true };

  Mask mask;

  [[nodiscard]] inline int bottom() const
  {
    return mask.bottom(y);
  }

  [[nodiscard]] inline int top() const
  {
    return mask.top(y);
  }

  [[nodiscard]] inline int left() const
  {
    return mask.left(x);
  }

  [[nodiscard]] inline int right() const
  {
    return mask.right(x);
  }

  [[nodiscard]] bool is_colliding_with_any(int offset_x = 0, int offset_y = 0) const;
  [[nodiscard]] bool is_colliding_with_nonsolid(int offset_x = 0, int offset_y = 0) const;
  [[nodiscard]] bool is_colliding_with_solid(int offset_x = 0, int offset_y = 0) const;

  [[nodiscard]] bool is_standing() const;
  [[nodiscard]] bool mask_free(Mask mask, int offset_x = 0, int offset_y = 0);

  [[nodiscard]] EntitySet get_colliding_solid(int offset_x = 0, int offset_y = 0) const;
  [[nodiscard]] EntitySet get_colliding_nonsolid(int offset_x = 0, int offset_y = 0) const;
  [[nodiscard]] EntitySet get_colliding_any(int offset_x = 0, int offset_y = 0) const;

  [[nodiscard]] std::set<Physics *> get_riding() const;
  [[nodiscard]] std::set<Physics *> get_pushable() const;

  void update_previous();

private:
  void check_collisions();
  void update_movement();
  void update_physics();

  void move_xy(float vec_x, float vec_y);

  [[nodiscard]] inline bool is_colliding(const Physics &other, int offset_x, int offset_y) const
  {
    if (mask.width == 0 || mask.height == 0)
      return false;

    const auto &my_rect    = mask.rect(x + offset_x, y + offset_y);
    const auto &other_rect = other.mask.rect(other.x, other.y);

    return CheckCollisionRecs(my_rect, other_rect);
  }
};

EXTERN_COMPONENT_TEMPLATE(Physics);
