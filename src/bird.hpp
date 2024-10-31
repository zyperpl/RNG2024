#pragma once

#include "manager.hpp"
#include "component.hpp"
#include "particles.hpp"
#include "level.hpp"

struct Bird
{
  COMPONENT(Bird);
  Bird(const Level::Entity &entity);

  void init();
  void preupdate();
  void postupdate();

private:
  Particle particle;
  int start_x { 0 };
  int start_y { 0 };

  int visible_timer { 0 };
  int visible_timer_max { 180 };
};

EXTERN_COMPONENT_TEMPLATE(Bird);
