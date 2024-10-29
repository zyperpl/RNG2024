#pragma once

#include "component.hpp"
#include "manager.hpp"
#include "particles.hpp"
#include "sound.hpp"

struct Bullet
{
  COMPONENT(Bullet);
  Bullet(int x, int y, float vx, float vy)
    : start_x{ x }
    , start_y{ y }
    , initial_v{ vx, vy }
  {
  }

  void init();
  void update();
  void postupdate();
  void collision(Entity other);

private:
  Particle trail_particle;
  Particle hit_particle;
  GameSound sound;
  int start_x{ 0 };
  int start_y{ 0 };
  Vector2 initial_v{ 0, 0 };
  int life{ 120 };
};

EXTERN_COMPONENT_TEMPLATE(Bullet);
