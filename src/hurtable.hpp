#pragma once

#include "component.hpp"
#include "manager.hpp"

struct Hurtable
{
  COMPONENT(Hurtable);

  void update()
  {
    invincibility_frames = std::max(0, invincibility_frames - 1);
  }

  inline bool process()
  {
    if (hurt_timer-- <= 0)
      return false;

    return true;
  }

  inline bool is_dead() const
  {
    return health <= 0;
  }

  inline bool hurt(int damage, int point_x, int point_y)
  {
    if (invincibility_frames > 0)
      return false;

    health               = std::max(0, health - damage);
    hit_point_x          = point_x;
    hit_point_y          = point_y;
    hurt_timer           = 1;
    invincibility_frames = invincibility_frames_max;

    return true;
  }

  inline int hurt_frames_ago() const
  {
    return -(hurt_timer - 1);
  }

  inline void reset()
  {
    health = max_health;
  }

  inline void add_health(int amount)
  {
    health = std::min(max_health, health + amount);
  }

  inline void set_max_health(int new_max)
  {
    max_health = new_max;
    health     = new_max;
  }

  inline void set_max_invincibility(int frames)
  {
    invincibility_frames_max = frames;
  }

  inline bool is_invincible() const
  {
    return invincibility_frames > 0;
  }

  int hit_point_x{ 0 };
  int hit_point_y{ 0 };

  int max_health{ 10 };
  int health{ max_health };

private:
  int hurt_timer{ 0 };
  int invincibility_frames{ 0 };
  int invincibility_frames_max{ 120 };
};

EXTERN_COMPONENT_TEMPLATE(Hurtable);
