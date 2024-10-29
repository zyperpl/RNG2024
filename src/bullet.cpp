#include "bullet.hpp"

#include "game.hpp"
#include "particles.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "sound.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Bullet);
COMPONENT_TEMPLATE(Bullet);

void Bullet::init()
{
  auto &physics = add_component(entity, Physics()).get();
  physics.x = start_x;
  physics.y = start_y;
  physics.v = initial_v;
  physics.solid = false;
  physics.mask  = Mask::center_rect(8, 8);

  auto &renderer = add_component(entity, SpriteRenderer("assets/tileset.png")).get();
  auto &sprite   = renderer.sprite_interpolated.sprite;
  sprite.set_frame_count(1);
  sprite.source_offset.x = 0;
  sprite.source_offset.y = 216;
  sprite.set_frame_width(8);
  sprite.set_frame_height(8);
  sprite.scale.x = 0.75f;
  sprite.scale.y = 0.75f;
  sprite.set_centered();

  trail_particle = Game::particle_builder()
                     .size(0.5f)
                     .life(10, 20)
                     .color(PALETTE_GRAY)
                     .velocity({ 0, 0 }, { -0.5f, -0.5f }, { 0.5f, 0.5f })
                     .sprite("assets/tileset.png_dust")
                     .build();

  hit_particle = Game::particle_builder()
                   .size(1.0f)
                   .life(10, 20)
                   .color(PALETTE_RED)
                   .velocity({ 0, 0 }, { -0.5f, -0.5f }, { 0.5f, 0.5f })
                   .sprite("assets/tileset.png_dust")
                   .build();
}
void Bullet::update()
{
  auto &physics = get_component<Physics>(entity).get();
  if (chance(50))
    Game::add_particles(physics.x, physics.y, trail_particle, 1);
}

void Bullet::postupdate()
{
  auto &physics  = get_component<Physics>(entity).get();
  auto &renderer = get_component<SpriteRenderer>(entity).get();

  renderer.set_position(physics.x, physics.y);

  const auto &level_width = Game::level_width();
  const auto &level_height = Game::level_height();

  life = std::max(life - 1, 0);

  if (physics.x < 0 || physics.x > level_width || physics.y < 0 || physics.y > level_height)
    destroy_entity(entity);

  if (life <= 0)
  {
    Game::add_particles(physics.x, physics.y, hit_particle, 10);
    destroy_entity(entity);
  }
}

void Bullet::collision(Entity other)
{
  auto other_physics_ref = get_component<Physics>(other);
  if (!other_physics_ref)
    return;

  auto &physics       = get_component<Physics>(entity).get();
  auto &other_physics = other_physics_ref.get();
  if (other_physics.solid)
  {
    Game::add_particles(physics.x, physics.y, hit_particle, 10);
    destroy_entity(entity);
  }
}