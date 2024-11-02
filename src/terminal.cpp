#include "terminal.hpp"

#include "game.hpp"
#include "interactable.hpp"
#include "level.hpp"
#include "light.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "sound.hpp"
#include "utils.hpp"

#include "magic_enum.hpp"

REGISTER_COMPONENT(Terminal);
COMPONENT_TEMPLATE(Terminal);
REGISTER_LEVEL_ENTITY(Terminal);

Terminal::Terminal(const Level::Entity &entity)
{
  start_x = entity.position.x;
  start_y = entity.position.y;

  w = entity.size.w;
  h = entity.size.h;

  std::string type_str;
  read_field(entity.fields, "Type", type_str);
  auto type_val = magic_enum::enum_cast<Type>(type_str);
  if (type_val)
    type = type_val.value();
  else
  {
    assert(false && "Unknown terminal type");
  }

  std::string text_str;
  read_field(entity.fields, "Text", text_str);
  messages = split(text_str, '\n');
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

  add_component(entity, Light(physics.x + 12, physics.y + 5));

  {
    auto &interactable = add_component(entity, Interactable()).get();

    if (!messages.empty())
    {
      for (const auto &message : messages)
      {
        printf("Adding message: %s\n", message.c_str());
        interactable.add(Interactable::ActionMessage{ message });
      }
    }

    if (type == Type::EndLevel)
    {
      interactable.add(Interactable::ActionEndLevel());
    }
  }

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
    disable();
  }
}

void Terminal::disable()
{
  auto &interactable = get_component<Interactable>(entity).get();
  auto &light        = get_component<Light>(entity).get();

  light.intensity = 0.1f;
  light.size      = 0.8f;

  interactable.disable();

  auto &renderer                               = get_component<SpriteRenderer>(entity).get();
  renderer.sprite_interpolated.animation_speed = 0;
  auto &sprite                                 = renderer.sprite_interpolated.sprite;
  sprite.set_frame(1);
}
