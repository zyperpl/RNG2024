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
    spr.set_frame(2);
    spr.set_centered();
  }

  {
    barrel              = add_component(entity, SpriteRenderer("assets/tileset.png"));
    auto &renderer      = barrel.get();
    auto &spr           = renderer.sprite_interpolated.sprite;
    spr.source_offset.x = 0;
    spr.source_offset.y = 232 - 8;
    spr.set_frame_width(8);
    spr.set_frame_height(8);
    spr.set_frame_count(1);
    spr.set_centered();
    spr.origin.x -= 8;
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
    dir_y = -1;
  }
  else
  {
    dir_y = 0;
  }

  if (input.jump)
  {
    if (physics.is_standing())
    {
      physics.v.y = -jump;
    }
  }
}

void Player::postupdate()
{
  auto &physics = get_component<Physics>(entity).get();

  const int y_bump_offset = physics.x / 12 % 2 == 0;
  if (body)
  {
    body.get().set_position(physics.x, physics.y - y_bump_offset);
    if (dir_x != 0)
      body.get().sprite_interpolated.sprite.scale.x = dir_x;
  }

  if (wheel1 && wheel2)
  {
    const float y_suspension_offset = std::max(std::min(3.0f, -physics.v.y * 1.2f), -3.0f);
    wheel1.get().set_position(physics.x - 8.0f, physics.y + 5.0f + y_suspension_offset);
    wheel2.get().set_position(physics.x + 8.0f, physics.y + 5.0f + y_suspension_offset);

    auto &wheel1_spr = wheel1.get().sprite_interpolated;
    auto &wheel2_spr = wheel2.get().sprite_interpolated;

    if (fabs(physics.v.x) > 0.5f)
    {
      wheel1_spr.animation_speed = dir_x;
      wheel2_spr.animation_speed = dir_x;
    }
    else
    {
      wheel1_spr.animation_speed = 0;
      wheel2_spr.animation_speed = 0;
    }
  }

  if (barrel)
  {
    barrel.get().set_position(physics.x + 0.0f, physics.y - 4.0f + 1.0f - y_bump_offset);

    auto &barrel_spr = barrel.get().sprite_interpolated;

    if (dir_x != 0)
    {
      barrel_spr.sprite.scale.x  = dir_x;
      barrel_spr.sprite.origin.x = 4 - 8 * dir_x;
    }

    if (dir_y == -1)
    {
      const float target_rotation = -90.0f * dir_x;
      if (fabs(target_rotation - barrel_spr.sprite.rotation) > 90.0f)
        barrel_spr.sprite.rotation = target_rotation;

      barrel_spr.sprite.rotation = lerp(barrel_spr.sprite.rotation, target_rotation, 0.6f);
    }
    else
    {
      barrel_spr.sprite.rotation = lerp(barrel_spr.sprite.rotation, 0.0f, 0.6f);
    }
  }
}
