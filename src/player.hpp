#pragma once

#include "manager.hpp"

#include "particles.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "utils.hpp"

struct Player
{
  COMPONENT(Player);

  void init();
  void preupdate();
  void update();
  void postupdate();

private:
  int dir_x{ 1 };
  int dir_y{ 0 };

  ComponentReference<SpriteRenderer> body;
  ComponentReference<SpriteRenderer> wheel1;
  ComponentReference<SpriteRenderer> wheel2;
  ComponentReference<SpriteRenderer> barrel;
};

EXTERN_COMPONENT_TEMPLATE(Player);
