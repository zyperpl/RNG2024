#pragma once

#include "component.hpp"
#include "hurtable.hpp"
#include "level.hpp"
#include "manager.hpp"
#include "particles.hpp"
#include "sound.hpp"

struct Enemy
{
  enum class Type
  {
    Slime,
    Bat,
    FallSlime,
    Boss
  };

  COMPONENT(Enemy);

  Enemy(int x, int y, Enemy::Type);
  Enemy(const Level::Entity &entity);

  void init();
  void update();
  void postupdate();

private:
  Particle hurt_particle;
  Particle hurt_particle2;
  Particle death_particle;
  int start_x{ 0 };
  int start_y{ 0 };
  int dir_x{ -1 };
  bool alerted{ false };
  LevelEntityId level_entity_id;
  Vector2 target{ 0, 0 };
  Type type{ Type::Slime };
  int shoot_timer{ 0 };
  int shoot_max_timer{ 80 };
};

EXTERN_COMPONENT_TEMPLATE(Enemy);
