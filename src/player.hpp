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

  int jump_buffer { 0 };
  static constexpr int JUMP_BUFFER_MAX = 10;
  int standing_buffer { 0 };
  static constexpr int STANDING_BUFFER_MAX = 6;
  int landed { 0 };
  static constexpr int LANDED_MAX = 20;

  ComponentReference<SpriteRenderer> body;
  ComponentReference<SpriteRenderer> wheel1;
  ComponentReference<SpriteRenderer> wheel2;
  ComponentReference<SpriteRenderer> barrel;
};

EXTERN_COMPONENT_TEMPLATE(Player);
