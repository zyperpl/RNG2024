#pragma once

#include "component.hpp"
#include "level.hpp"
#include "manager.hpp"

struct Light
{
  COMPONENT(Light);
  Light(const Level::Entity &entity);

  void init();

  int start_x{ 0 };
  int start_y{ 0 };
  int x{ 0 };
  int y{ 0 };
  float size{ 1.0f };
  float intensity{ 1.0f };

private:
};

EXTERN_COMPONENT_TEMPLATE(Light);
