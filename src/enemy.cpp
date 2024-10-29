#include "enemy.hpp"

#include "game.hpp"
#include "level.hpp"
#include "light.hpp"
#include "particles.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "sound.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Enemy);
COMPONENT_TEMPLATE(Enemy);
REGISTER_LEVEL_ENTITY(Enemy);

Enemy::Enemy(const Level::Entity &entity)
{
  start_x = entity.position.x + entity.size.w / 2;
  start_y = entity.position.y + entity.size.h / 2;
}

void Enemy::init()
{
  auto &physics   = add_component(entity, Physics()).get();
  physics.mask    = Mask::center_rect(16, 16);
  physics.x       = start_x;
  physics.y       = start_y;
  physics.gravity = 0.1f;
  physics.v.x     = chance(90) ? -0.5f : 0.5f;

  auto &renderer = add_component(entity, SpriteRenderer("assets/tileset.png")).get();
  auto &sprite   = renderer.sprite_interpolated.sprite;
  sprite.set_frame_width(16);
  sprite.set_frame_height(16);
  sprite.source_offset.x = 0;
  sprite.source_offset.y = 192;
  sprite.set_frame_durations({ 200, 100 });
  sprite.set_frame_count(2);
  sprite.set_frame(0);
  sprite.set_centered();

  add_component(entity, Hurtable());

  hurt_particle = Game::particle_builder()
                    .size(0.5f)
                    .life(30, 30)
                    .color(PALETTE_RED)
                    .velocity({ -0.1f, -0.1f }, { 0.1f, 0.0f })
                    .sprite("assets/tileset.png:star")
                    .build();

  death_particle = Game::particle_builder()
                     .size(1.0f)
                     .life(40, 80)
                     .color(PALETTE_RED, PALETTE_GRAY)
                     .velocity({ -0.4f, -0.4f }, { 0.4f, 0.4f })
                     .sprite("assets/tileset.png:dust")
                     .build();

  add_component(entity, Light());
}

void Enemy::update()
{
  auto &physics = get_component<Physics>(entity).get();

  if (physics.is_standing())
  {
    int jump_prob = 1;
    if (fabs(physics.v.x) < 0.1f)
      jump_prob = 30;

    if (chance(jump_prob))
    {
      physics.v.y = -3.0f;
    }
  }

  physics.v.x = -0.5f;

  dir_x = physics.v.x > 0 ? 1 : -1;
}

void Enemy::postupdate()
{
  auto &physics  = get_component<Physics>(entity).get();
  auto &light    = get_component<Light>(entity).get();
  auto &renderer = get_component<SpriteRenderer>(entity).get();
  renderer.set_position(physics.x, physics.y);

  if (fabs(physics.v.x) < 0.5f)
    renderer.sprite_interpolated.animation_speed = 0;
  else
    renderer.sprite_interpolated.animation_speed = 1;

  if (dir_x != 0)
    renderer.sprite_interpolated.sprite.scale.x = static_cast<float>(dir_x);

  auto &hurtable = get_component<Hurtable>(entity).get();
  if (hurtable.process())
  {
    light.intensity = 1.0f;
    Game::add_particles(physics.x, physics.y, hurt_particle, 2);

    if (hurtable.is_dead())
    {
      light.size      = 2.0f;
      light.intensity = 5.0f;
      for (int i = 0; i < 10; i++)
      {
        const int part_x = physics.x + randi(-8, 8);
        const int part_y = physics.y + randi(-8, 8);
        Game::add_particles(part_x, part_y, death_particle, 2);
      }
      renderer.sprite_interpolated.visible = false;
      physics.collidable = false;
      physics.solid = false;
      physics.do_update = false;

      Game::add_timer(entity, [&] { destroy_entity(entity); }, 30);
    }
  }

  light.x         = physics.x;
  light.y         = physics.y;
  light.size      = lerp(light.size, 1.0f, 0.1f);
  light.intensity = lerp(light.intensity, 0.0f, 0.2f);
}
