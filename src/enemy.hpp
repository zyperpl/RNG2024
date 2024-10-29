#pragma once

#include "component.hpp"
#include "hurtable.hpp"
#include "level.hpp"
#include "manager.hpp"
#include "particles.hpp"
#include "sound.hpp"

struct Enemy
{
  COMPONENT(Enemy);
  Enemy(const Level::Entity &entity);

  void init();
  void update();
  void postupdate();

private:
  Particle hurt_particle;
  Particle death_particle;
  int start_x{ 0 };
  int start_y{ 0 };
  int dir_x { -1 };
};

EXTERN_COMPONENT_TEMPLATE(Enemy);
