#pragma once

#include <chrono>

#include "manager.hpp"

#include "particles.hpp"
#include "physics.hpp"
#include "utils.hpp"
#include "sound.hpp"

struct Light;
struct SpriteRenderer;

struct Player
{
  COMPONENT(Player);

  void init();
  void preupdate();
  void update();
  void postupdate();
  void collision(Entity other);

  std::chrono::time_point<std::chrono::high_resolution_clock> init_time;

  GameSound boom_sound;
private:
  int dir_x{ 1 };
  int dir_y{ 0 };

  int jump_buffer{ 0 };
  static constexpr int JUMP_BUFFER_MAX = 10;
  int standing_buffer{ 0 };
  static constexpr int STANDING_BUFFER_MAX = 6;
  int landed{ 0 };
  static constexpr int LANDED_MAX = 20;
  int shoot_cooldown{ 0 };
  static constexpr int SHOOT_COOLDOWN_MAX = 16;

  ComponentReference<SpriteRenderer> body;
  ComponentReference<SpriteRenderer> wheel1;
  ComponentReference<SpriteRenderer> wheel2;
  ComponentReference<SpriteRenderer> barrel;
  ComponentReference<Light> light;
  Particle shoot_particle;
  Particle jump_particle;

  GameSound shoot_sound;
  GameSound jump_sound;
  GameSound land_sound;
  GameSound hurt_sound;

  bool can_interact{ false };
  Vector2 interact_point{ 0.0f, 0.0f };

  friend struct Game;
};

EXTERN_COMPONENT_TEMPLATE(Player);
