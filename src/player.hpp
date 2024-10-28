#pragma once

#include "manager.hpp"

#include "particles.hpp"
#include "physics.hpp"
#include "utils.hpp"

struct Player
{
  COMPONENT(Player);

  void init();
  void preupdate();
  void update();
  void postupdate();
  void render();

private:
};

EXTERN_COMPONENT_TEMPLATE(Player);
