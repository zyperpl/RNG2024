#include "battery.hpp"

#include "enemy.hpp"
#include "game.hpp"
#include "hurtable.hpp"
#include "interactable.hpp"
#include "level.hpp"
#include "light.hpp"
#include "particles.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "sound.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Battery);
COMPONENT_TEMPLATE(Battery);

REGISTER_LEVEL_ENTITY(Battery);

Battery::Battery(const Level::Entity &entity)
{
  start_x = entity.position.x;
  start_y = entity.position.y;

  Level::read_field(entity.fields, "Identifier", level_entity_id);
}

void Battery::init()
{
  auto &physics = add_component(entity, Physics()).get();
  physics.mask  = Mask::center_rect(16, 12);
  physics.mask.origin_y -= 2;
  physics.x     = start_x;
  physics.y     = start_y + 8;
  physics.solid = true;

  auto &renderer         = add_component(entity, SpriteRenderer("assets/tileset.png")).get();
  auto &sprite           = renderer.sprite_interpolated.sprite;
  sprite.source_offset.x = 56;
  sprite.source_offset.y = 88;
  sprite.set_frame_count(1);
  sprite.set_frame_width(16);
  sprite.set_frame_height(16);
  sprite.set_centered();

  particle = Game::particle_builder()
               .size(0.5f, 0.8f, -0.01f)
               .life(60, 60)
               .color(PALETTE_GREEN)
               .velocity({ -0.3f, -0.3f }, { 0.3f, 0.3f })
               .gravity(0.01f)
               .sprite("assets/tileset.png:bit")
               .build();

  particle2 = Game::particle_builder()
                .size(0.7f, 1.0f)
                .life(60, 160)
                .color(PALETTE_YELLOW)
                .velocity({ -0.5f, -0.5f }, { 0.5f, 0.5f })
                .gravity(-0.01f)
                .sprite("assets/tileset.png:dust")
                .build();

  particle3 = Game::particle_builder()
                .size(0.7f, 0.9f)
                .life(20, 60)
                .color(PALETTE_WHITE)
                .velocity({ -1.0f, -1.0f }, { 1.0f, 1.0f })
                .gravity(-0.01f)
                .sprite("assets/tileset.png:star")
                .build();

  // sound = GameSound();

  Level::Field collected_field;
  Level::Level::load(level_entity_id, "Collected", collected_field);
  if (auto collected = std::get_if<bool>(&collected_field); collected && *collected)
  {
    used = true;
    destroy_entity(entity);
  }

  auto &light     = add_component(entity, Light(physics.x, physics.y)).get();
  light.size      = 0.1f;
  light.intensity = 1.4f;

  auto &hurtable  = add_component(entity, Hurtable()).get();
  hurtable.health = 3;
  hurtable.set_max_invincibility(1);

  auto &interactable = add_component(entity, Interactable()).get();
  interactable.add(Interactable::HealthPickup(5));
}

void Battery::postupdate()
{
  auto &renderer = get_component<SpriteRenderer>(entity).get();
  auto &physics  = get_component<Physics>(entity).get();
  renderer.set_position(physics.x, physics.y);

  if (used)
  {
    auto &scale = renderer.sprite_interpolated.sprite.scale;
    if (scale.x > 0.0f)
    {
      scale.x = lerp(scale.x, 0.0f, 0.1f);
      scale.y = lerp(scale.y, 0.0f, 0.1f);
      renderer.sprite_interpolated.sprite.set_centered();

      if (physics.y < start_y + 12)
        physics.v.y = 0.5f;
      else
        physics.v.y = 0.0f;
    }
    return;
  }

  auto &light        = get_component<Light>(entity).get();
  auto &hurtable     = get_component<Hurtable>(entity).get();
  auto &interactable = get_component<Interactable>(entity).get();

  if (hurtable.process())
  {
    light.intensity = 10.0f;
    Game::add_particles(physics.x, physics.y, particle, 10);
    Game::skip_ticks(1);

    if (hurtable.is_dead() && !interactable.is_used())
    {
      used = true;

      light.intensity = 10.0f;
      light.size      = 2.0f;
      Level::Level::store(level_entity_id, "Collected", true);

      for (int i = 0; i < 10; i++)
      {
        int px = physics.left() + randi(-6, physics.mask.width + 6);
        int py = physics.top() + randi(-8, physics.mask.height);
        Game::add_particles(px, py, particle2, 2);
        Game::add_particles(px, py, particle3, 3);
      }

      for (auto &enemy : get_components<Enemy>())
      {
        auto enemy_hurtable_ref = get_component<Hurtable>(enemy.entity);
        if (enemy_hurtable_ref)
        {
          auto &enemy_hurtable = enemy_hurtable_ref.get();
          enemy_hurtable.hurt(5, physics.x, physics.y);
        }
      }

      Game::add_timer(entity, [entity = entity] { destroy_entity(entity); }, 10);
    }
  }

  if (interactable.is_used())
  {
    used = true;

    interactable.disable();
    light.intensity = 0.5f;
    for (int i = 0; i < 10; i++)
    {
      int px = physics.left() + randi(0, physics.mask.width);
      int py = physics.top() + randi(0, physics.mask.height);
      Game::add_particles(px, py, particle, 10);
    }
    Game::add_timer(entity, [entity = entity] { destroy_entity(entity); }, 30);
    Level::Level::store(level_entity_id, "Collected", true);
  }

  light.intensity = lerp(light.intensity, 1.4f, 0.1f);
}
