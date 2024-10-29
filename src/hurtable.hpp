#pragma once

#include "component.hpp"
#include "manager.hpp"

struct Hurtable
{
  COMPONENT(Hurtable);

  inline bool process()
  {
    if (hurt_timer <= 0)
      return false;

    hurt_timer = std::max(0, hurt_timer - 1);
    return true;
  }

  inline bool is_dead() const
  {
    return health <= 0;
  }

  inline void hurt(int damage)
  {
    health     = std::max(0, health - damage);
    hurt_timer = 1;
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
    health     = std::min(max_health, health);
  }

private:
  int max_health{ 100 };
  int health{ max_health };
  int hurt_timer{ 0 };
};

EXTERN_COMPONENT_TEMPLATE(Hurtable);
