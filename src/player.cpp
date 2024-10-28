#include "player.hpp"

#include "block.hpp"
#include "game.hpp"
#include "input.hpp"
#include "level.hpp"
#include "manager.hpp"
#include "particles.hpp"
#include "physics.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Player);
COMPONENT_TEMPLATE(Player);

void Player::init()
{
  set_persistent(entity);

  auto &physics = add_component(entity, Physics()).get();
  physics.mask  = Mask::center_rect(12, 28);
}

void Player::preupdate() {}

void Player::update()
{
  auto &physics = get_component<Physics>(entity).get();
  auto &input   = Input::get();

  const auto speed = 4.0f;
  if (input.left)
    physics.v.x = -speed;
  else if (input.right)
    physics.v.x = speed;
  else
    physics.v.x = 0;

  if (input.up)
    physics.v.y = -speed;
  else if (input.down)
    physics.v.y = speed;
  else
    physics.v.y = 0;
}

void Player::postupdate() {}

void Player::render()
{
  const auto &physics = get_component<Physics>(entity).get();

  DrawRectangleRounded(physics.mask.rect(physics.x, physics.y), 0.5, 4, WHITE);
}
