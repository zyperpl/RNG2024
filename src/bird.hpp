#pragma once

#include "component.hpp"
#include "level.hpp"
#include "manager.hpp"
#include "particles.hpp"

struct Bird
{
  COMPONENT(Bird);
  Bird(const Level::Entity &entity);

  void init();
  void preupdate();
  void postupdate();

private:
  Particle particle;
  int start_x{ 0 };
  int start_y{ 0 };

  int visible_timer{ 0 };
  int visible_timer_max{ 60 };
};

EXTERN_COMPONENT_TEMPLATE(Bird);
