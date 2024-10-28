#include "player.hpp"

#include "block.hpp"
#include "game.hpp"
#include "input.hpp"
#include "level.hpp"
#include "manager.hpp"
#include "particles.hpp"
#include "physics.hpp"
#include "renderers.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Player);
COMPONENT_TEMPLATE(Player);

void Player::init()
{
  set_persistent(entity);

  auto &physics   = add_component(entity, Physics()).get();
  physics.x       = 60;
  physics.y       = 80;
  physics.mask    = Mask::center_rect(22, 16);
  physics.gravity = 0.2f;

  {
    body                = add_component(entity, SpriteRenderer("assets/tileset.png"));
    auto &renderer      = body.get();
    auto &spr           = renderer.sprite_interpolated.sprite;
    spr.source_offset.x = 0;
    spr.source_offset.y = 232;
    spr.set_frame_width(24);
    spr.set_frame_height(16);
    spr.set_frame_count(1);
    spr.set_centered();
  }

  {
    wheel1              = add_component(entity, SpriteRenderer("assets/tileset.png"));
    auto &renderer      = wheel1.get();
    auto &spr           = renderer.sprite_interpolated.sprite;
    spr.source_offset.x = 0;
    spr.source_offset.y = 232 + 16;
    spr.set_frame_width(8);
    spr.set_frame_height(8);
    spr.set_frame_count(4);
    spr.set_centered();
  }

  {
    wheel2              = add_component(entity, SpriteRenderer("assets/tileset.png"));
    auto &renderer      = wheel2.get();
    auto &spr           = renderer.sprite_interpolated.sprite;
    spr.source_offset.x = 0;
    spr.source_offset.y = 232 + 16;
    spr.set_frame_width(8);
    spr.set_frame_height(8);
    spr.set_frame_count(4);
    spr.set_centered();
  }
}

void Player::preupdate() {}

void Player::update()
{
  auto &physics = get_component<Physics>(entity).get();
  auto &input   = Input::get();

  const auto speed = 1.0f;
  const auto jump  = 3.0f;
  if (input.left)
  {
    physics.v.x = -speed;
    dir_x       = -1;
  }
  else if (input.right)
  {
    physics.v.x = speed;
    dir_x       = 1;
  }
  else
  {
    physics.v.x *= 0.8f;
  }

  if (input.up)
  {
    if (physics.is_standing())
    {
      physics.v.y = -jump;
    }
    dir_y = -1;
  }
  else if (input.down)
  {
    dir_y = 1;
  }
  else
  {
    dir_y = 0;
  }
}

void Player::postupdate()
{
  auto &physics  = get_component<Physics>(entity).get();
  const int y_bump_offset = physics.x / 12 % 2 == 0;
  body.get().set_position(physics.x, physics.y - y_bump_offset);
  if (dir_x != 0)
    body.get().sprite_interpolated.sprite.scale.x = dir_x;

  const float y_suspension_offset = std::max(std::min(3.0f, -physics.v.y * 1.2f), -3.0f);
  wheel1.get().set_position(physics.x - 8.0f, physics.y + 5.0f + y_suspension_offset);
  wheel2.get().set_position(physics.x + 8.0f, physics.y + 5.0f + y_suspension_offset);

  if (fabs(physics.v.x) > 0.5f)
  {
    wheel1.get().sprite_interpolated.animation_speed = dir_x;
    wheel2.get().sprite_interpolated.animation_speed = dir_x;
  } 
  else
  {
    wheel1.get().sprite_interpolated.animation_speed = 0;
    wheel2.get().sprite_interpolated.animation_speed = 0;
  }
}
