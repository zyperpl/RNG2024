#pragma once

#include "component.hpp"
#include "level.hpp"
#include "manager.hpp"
#include "particles.hpp"
#include "sound.hpp"

struct Battery
{
  COMPONENT(Battery);
  Battery(const Level::Entity &entity);

  void init();
  void postupdate();

private:
  Particle particle;
  Particle particle2;
  Particle particle3;
  GameSound sound;

  int start_x{ 0 };
  int start_y{ 0 };
  bool used { false };
  LevelEntityId level_entity_id;
};

EXTERN_COMPONENT_TEMPLATE(Battery);
