#pragma once

#include "manager.hpp"
#include "component.hpp"
#include "level.hpp"

struct Light
{
  COMPONENT(Light);
  Light(const Level::Entity &entity);

  void init();

  int start_x { 0 };
  int start_y { 0 };
  float strength { 1.0f };
private:
};

EXTERN_COMPONENT_TEMPLATE(Light);
