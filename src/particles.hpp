#pragma once

#include "component.hpp"
#include "manager.hpp"

#include "sprite.hpp"

struct Particle;
struct ParticleBuilder;
struct ParticleSystem;

struct Particle
{
  float x{ 0.0f };
  float y{ 0.0f };

  Vector2 v{ 0.0f, 0.0f };
  Vector2 v_min{ 0.0f, 0.0f };
  Vector2 v_max{ 0.0f, 0.0f };
  Vector2 v_inc{ 0.0f, 0.0f };

  float size{ 1.0f };
  float size_incr{ 0.0f };
  float size_min{ 1.0f };
  float size_max{ 1.0f };

  uint8_t life{ 1 };
  uint8_t life_min{ 1 };
  uint8_t life_max{ 1 };
  uint8_t start_life{ 0 };

  float gravity{ 0.0f };

  Color color{ BLACK };
  float alpha{ 1.0f };

  float alpha1{ 1.0f };
  float alpha2{ 1.0f };
  float alpha3{ 1.0f };
  Color color1{ WHITE };
  Color color2{ WHITE };
  Color color3{ WHITE };

  size_t sprite_id{ std::numeric_limits<size_t>::max() };
  float frame{ 0.0f };
  float frame_incr{ 0.0f };
  uint8_t frame_min{ 0 };
  uint8_t frame_max{ 0 };

  inline bool is_sprite_loaded() const
  {
    return sprite_id != std::numeric_limits<size_t>::max();
  }
};

struct ParticleBuilder
{
  [[nodiscard]] constexpr inline ParticleBuilder(ParticleSystem &system)
    : system(system)
  {
  }

  ParticleSystem &system;
  Particle part;

  ParticleBuilder &size(float size);
  ParticleBuilder &size(float size_min, float size_max, float size_incr = 0.0f);
  ParticleBuilder &life(int life_min, int life_max);
  ParticleBuilder &alpha(float alpha_beg, float alpha_mid, float alpha_end);
  ParticleBuilder &alpha(float alpha_beg, float alpha_end);
  ParticleBuilder &alpha(float alpha);
  ParticleBuilder &color(Color color_beg, Color color_mid, Color color_end);
  ParticleBuilder &color(Color color_beg, Color color_end);
  ParticleBuilder &color(Color color);
  ParticleBuilder &gravity(float gravity);
  ParticleBuilder &velocity(Vector2 v);
  ParticleBuilder &velocity(Vector2 v_min, Vector2 v_max);
  ParticleBuilder &velocity(Vector2 v, Vector2 v_min, Vector2 v_max);
  ParticleBuilder &sprite(const std::string &filename);
  ParticleBuilder &sprite(const std::string &filename, float frame_incr);
  [[nodiscard]] Particle build();
};

struct ParticleSystem
{
  COMPONENT(ParticleSystem);

  ParticleSystem(int depth)
    : depth(depth)
  {
  }

  void init()
  {
    set_persistent(entity);
  }
  void add_particle(int x, int y, const Particle &type);
  void add_particles(int x, int y, const Particle &type, int count);

  void update();
  void render();

  std::vector<Particle> particles;
  void clear();

  size_t sprite_id(const std::string &filename);
  const Sprite &get_sprite(size_t id);

  int depth{ 0 };
  bool visible{ true };

private:
  std::vector<Sprite> sprites;
  std::unordered_map<std::string, size_t> sprite_ids;
  size_t add_sprite(const std::string &filename);
};

EXTERN_COMPONENT_TEMPLATE(ParticleSystem);
