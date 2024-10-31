#include "player.hpp"

#include "block.hpp"
#include "bullet.hpp"
#include "enemy.hpp"
#include "game.hpp"
#include "hurtable.hpp"
#include "input.hpp"
#include "interactable.hpp"
#include "level.hpp"
#include "light.hpp"
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

  auto &hurtable = add_component(entity, Hurtable()).get();
  hurtable.set_max_health(10);

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

  light = add_component(entity, Light());

  shoot_particle = Game::particle_builder()
                     .size(0.8f, 1.1f)
                     .life(10, 10)
                     .color(PALETTE_GRAY)
                     .velocity({ -0.1f, -0.1f }, { 0.1f, 0.1f })
                     .sprite("assets/tileset.png:dust")
                     .build();

  jump_particle = Game::particle_builder()
                    .size(0.5f)
                    .life(30, 30)
                    .color(PALETTE_BLUE)
                    .velocity({ -0.1f, -0.1f }, { 0.1f, 0.0f })
                    .sprite("assets/tileset.png:dust")
                    .build();
}

void Player::preupdate()
{
  assert(get_component<Physics>(entity) && "Player entity has no physics component");

  auto &physics = get_component<Physics>(entity).get();

  landed = std::max(landed - 1, 0);

  if (physics.is_standing())
  {
    if (standing_buffer <= 0)
    {
      landed = LANDED_MAX;

      jump_particle.v.x = physics.v.x * 0.2f + physics.v_previous.y * 0.1f;
      jump_particle.v.y = -0.1f;
      Game::add_particles(physics.x + 8, physics.bottom() + 2, jump_particle, 2);

      jump_particle.v.x = physics.v.x * 0.2f + -physics.v_previous.y * 0.1f;
      jump_particle.v.y = -0.1f;
      Game::add_particles(physics.x - 8, physics.bottom() + 2, jump_particle, 2);
    }

    standing_buffer = STANDING_BUFFER_MAX;
  }
  else
  {
    standing_buffer = std::max(standing_buffer - 1, 0);
  }
}

void Player::update()
{
  auto &physics = get_component<Physics>(entity).get();
  auto &input   = Input::get();

  can_interact                  = false;
  const Rectangle interact_rect = inflate(physics.mask.rect(physics.x, physics.y), 1.0f);
  for (auto &interactable : get_components<Interactable>())
  {
    if (interactable.can_interact(interact_rect))
    {
      can_interact = true;

      if (input.shoot.pressed())
        interactable.interact();
    }
  }

  const auto speed = landed <= 0 ? 1.4f : 1.2f;
  const auto jump  = 3.6f;
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
    physics.v.x *= 0.85f;
  }

  if (input.up)
    dir_y = -1;
  else
  {
    if (input.down)
    {
      dir_y  = 1;
      landed = 2;
    }
    else
      dir_y = 0;
  }

  if (input.jump.pressed())
    jump_buffer = JUMP_BUFFER_MAX;
  else
    jump_buffer = std::max(jump_buffer - 1, 0);

  if (jump_buffer && standing_buffer)
  {
    physics.v.y = -jump;

    jump_particle.v.x = 0.1f;
    jump_particle.v.y = -jump * 0.1f;
    Game::add_particles(physics.x - 4, physics.bottom(), jump_particle, 1);
    jump_particle.v.x = -0.1f;
    Game::add_particles(physics.x + 4, physics.bottom(), jump_particle, 1);

    jump_buffer     = 0;
    standing_buffer = 0;
  }

  if (!input.jump && physics.v.y < -1.0f)
    physics.v.y *= 0.9f;

  if (!can_interact && input.shoot && shoot_cooldown <= 0 && dir_y != 1)
  {
    shoot_cooldown = SHOOT_COOLDOWN_MAX;

    light.get().intensity = std::min(1.0f, light.get().intensity + 0.1f);
    light.get().x         = physics.x + (dir_y == 0 ? dir_x * 12.0f : 0.0f);
    light.get().y         = physics.y - (-dir_y * 12.0f);

    const int bullet_x = physics.x + (dir_y == 0 ? dir_x * 12.0f : 0.0f);
    const int bullet_y = physics.y - 5 - (-dir_y * 12.0f);
    add_entity(Bullet(entity, bullet_x, bullet_y, dir_y == 0 ? dir_x * 2.0f : 0.0f, dir_y * 2.0f));

    Game::add_particles(bullet_x, bullet_y, shoot_particle, 2);

    auto &manager = Manager::get();
    manager.call_init();

    if (!physics.is_standing())
      physics.v.x -= dir_x * 0.4f;
  }
  shoot_cooldown = std::max(shoot_cooldown - 1, 0);
}

void Player::postupdate()
{
  auto &physics  = get_component<Physics>(entity).get();
  auto &hurtable = get_component<Hurtable>(entity).get();

  const int y_bump_offset = (physics.x / 12 % 2 == 0) - std::min(landed / 2.0f, 2.0f) + 1.0f;
  if (body)
  {

    body.get().set_position(physics.x, physics.y - y_bump_offset);
    if (dir_x != 0)
    {
      auto &spr   = body.get().sprite_interpolated.sprite;
      spr.scale.x = lerp(spr.scale.x, dir_x, 0.8f);
    }

    auto &spr_inter   = body.get().sprite_interpolated;
    spr_inter.visible = hurtable.is_invincible() ? (Game::tick() / 3 % 2 == 0) : true;
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

  if (light)
  {
    light.get().x                 = lerp(light.get().x, physics.x, 0.6f);
    light.get().y                 = lerp(light.get().y, physics.y, 0.6f);
    const float DEFAULT_INTENSITY = 0.2f;
    light.get().intensity         = DEFAULT_INTENSITY;

    const float DEFAULT_SIZE = 1.0f;
    float target_size        = DEFAULT_SIZE;
    if (fabs(physics.v.x) > 1.0f)
      target_size = DEFAULT_SIZE + 2.0f * fabs(physics.v.x);
    light.get().size = lerp(light.get().size, target_size, 0.6f);
  }

  if (hurtable.process())
  {
    physics.v.x /= 2.0f;
    physics.v.y = -2.0f;
  }
}

void Player::collision(Entity other)
{
  auto &physics  = get_component<Physics>(entity).get();
  auto &hurtable = get_component<Hurtable>(entity).get();

  auto other_physics_ref = get_component<Physics>(other);

  if (auto enemy_ref = get_component<Enemy>(other))
  {
    int hit_point_x{ physics.x };
    int hit_point_y{ physics.y };

    if (other_physics_ref)
    {
      auto &other_physics = other_physics_ref.get();
      hit_point_x         = other_physics.x;
      hit_point_y         = other_physics.y;
    }

    hurtable.hurt(1, hit_point_x, hit_point_y);
  }
}
