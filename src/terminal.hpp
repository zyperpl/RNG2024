#pragma once

#include "component.hpp"
#include "level.hpp"
#include "manager.hpp"
#include "sound.hpp"

struct Terminal
{
  COMPONENT(Terminal);
  Terminal(const Level::Entity &entity);

  void init();
  void update();

private:
  GameSound sound;
  int start_x{ 0 };
  int start_y{ 0 };
  int w{ 8 };
  int h{ 8 };
};

EXTERN_COMPONENT_TEMPLATE(Terminal);
