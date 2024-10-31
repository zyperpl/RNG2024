#include "bird.hpp"

#include "game.hpp"
#include "level.hpp"
#include "particles.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Bird);
COMPONENT_TEMPLATE(Bird);
REGISTER_LEVEL_ENTITY(Bird);

Bird::Bird(const Level::Entity &entity)
{
  start_x = entity.position.x + entity.size.w / 2;
  start_y = entity.position.y + entity.size.h / 2;
}

void Bird::init()
{
  auto &physics = add_component(entity, Physics()).get();
  physics.mask  = Mask::center_rect(8, 8);
  physics.x     = start_x;
  physics.y     = start_y;

  auto &renderer = add_component(entity, SpriteRenderer("assets/tileset.png")).get();
  renderer.set_position(start_x, start_y);
  auto &sprite           = renderer.sprite_interpolated.sprite;
  sprite.source_offset.x = 0.0f;
  sprite.source_offset.y = 208.0f;
  sprite.set_frame_width(8);
  sprite.set_frame_height(8);
  sprite.set_frame_count(4);
  sprite.set_centered();
  sprite.set_frame_durations(200);
  sprite.add_tag("idle", { 0, 1 });
  sprite.add_tag("fly", { 2, 3 });
  sprite.set_tag("idle");
  sprite.set_frame_relative(randi(0, 1));
  sprite.scale.x = chance(50) ? 1.0f : -1.0f;

  particle = Game::particle_builder()
               .size(0.4f, 0.6f)
               .life(30, 60)
               .color(PALETTE_RED)
               .velocity({ -0.6f, -0.6f }, { 0.6f, 0.1f })
               .gravity(0.08f)
               .sprite("assets/tileset.png:bit")
               .build();
}

void Bird::preupdate()
{
  auto &physics = get_component<Physics>(entity).get();

  const auto rect = physics.mask.rect(physics.x, physics.y);

  if (visible_timer > 0 || Game::on_screen(rect))
    visible_timer += 1;

  if (visible_timer >= visible_timer_max)
  {
    physics.v.y -= 0.1f + randf(0.5f, 1.0f);
    physics.v.y = Clamp(physics.v.y, -4.0f, 4.0f);
    physics.v.x = randf(-2.0f, 2.0f);
  }
}

void Bird::postupdate()
{
  auto &renderer = get_component<SpriteRenderer>(entity).get();
  auto &physics  = get_component<Physics>(entity).get();
  renderer.set_position(physics.x, physics.y);

  if (fabs(physics.v.x) > 0.2f || fabs(physics.v.y) > 0.1f)
    renderer.sprite_interpolated.sprite.set_tag("fly");
  else
    renderer.sprite_interpolated.sprite.set_tag("idle");
}
