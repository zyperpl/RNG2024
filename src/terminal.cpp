#include "terminal.hpp"

#include "game.hpp"
#include "interactable.hpp"
#include "level.hpp"
#include "light.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "sound.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Terminal);
COMPONENT_TEMPLATE(Terminal);
REGISTER_LEVEL_ENTITY(Terminal);

Terminal::Terminal(const Level::Entity &entity)
{
  start_x = entity.position.x;
  start_y = entity.position.y;

  w = entity.size.w;
  h = entity.size.h;
}

void Terminal::init()
{
  auto &physics = add_component(entity, Physics()).get();
  physics.mask  = Mask::center_rect(w, h);
  physics.x     = start_x;
  physics.y     = start_y;
  physics.solid = false;

  auto &renderer = add_component(entity, SpriteRenderer("assets/tileset.png")).get();
  renderer.set_position(start_x, start_y);
  renderer.depth = 1;

  auto &sprite           = renderer.sprite_interpolated.sprite;
  sprite.source_offset.x = 0;
  sprite.source_offset.y = 72;
  sprite.set_frame_width(16);
  sprite.set_frame_height(32);
  sprite.set_frame_count(2);

  add_component(entity,
                Interactable(Interactable::ActionMessage{ "Not responding..." },
                             Interactable::ActionMessage{ "Hello world!", PALETTE_YELLOW }));

  add_component(entity, Light(physics.x + 12, physics.y + 5));
  // sound = GameSound();
}

void Terminal::update()
{
  auto &interactable = get_component<Interactable>(entity).get();

  if (!interactable.is_enabled())
    return;

  auto &light = get_component<Light>(entity).get();

  if (!interactable.is_used())
  {
    light.intensity = (sinf(Game::tick() * 0.05f) * 0.5f + 0.5f) * 0.6f;
    light.size      = (sinf(Game::tick() * 0.05f) * 0.5f + 0.5f) * 0.2f;
  }
  else
  {
    light.intensity = 0.1f;
    light.size      = 0.8f;

    interactable.disable();

    auto &renderer                               = get_component<SpriteRenderer>(entity).get();
    renderer.sprite_interpolated.animation_speed = 0;
    auto &sprite                                 = renderer.sprite_interpolated.sprite;
    sprite.set_frame(1);
  }
}
