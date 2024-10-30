#include "enemy.hpp"

#include "game.hpp"
#include "level.hpp"
#include "light.hpp"
#include "magic_enum.hpp"
#include "particles.hpp"
#include "physics.hpp"
#include "player.hpp"
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

  std::string type_str;
  try_read_field(entity.fields, "Type", type_str);

  if (auto type_val = magic_enum::enum_cast<Type>(type_str))
    this->type = type_val.value();
  else
    fprintf(stderr, "Unknown enemy type '%s'\n", type_str.c_str());
}

void Enemy::init()
{
  auto &physics = add_component(entity, Physics()).get();
  physics.mask  = Mask::center_rect(16, 16);
  physics.x     = start_x;
  physics.y     = start_y;
  physics.v.x   = chance(90) ? -0.5f : 0.5f;

  auto &hurtable = add_component(entity, Hurtable()).get();

  auto &renderer = add_component(entity, SpriteRenderer("assets/tileset.png")).get();
  auto &sprite   = renderer.sprite_interpolated.sprite;

  switch (type)
  {
    case Type::Slime:
      physics.gravity = 0.1f;

      hurtable.set_max_health(3);

      sprite.set_frame_width(16);
      sprite.set_frame_height(16);
      sprite.source_offset.x = 0;
      sprite.source_offset.y = 192;
      sprite.set_frame_durations({ 200, 100 });
      sprite.set_frame_count(2);

      break;
    case Type::Bat:
      physics.gravity = 0.01f;

      hurtable.set_max_health(2);

      sprite.set_frame_width(16);
      sprite.set_frame_height(16);
      sprite.source_offset.x = 0;
      sprite.source_offset.y = 192 - 16;
      sprite.set_frame_durations({ 200, 200 });
      sprite.set_frame_count(2);
      break;
  }

  sprite.set_frame(0);
  sprite.set_centered();

  hurt_particle = Game::particle_builder()
                    .size(0.5f)
                    .life(30, 30)
                    .color(PALETTE_RED)
                    .velocity({ -0.1f, -0.1f }, { 0.1f, 0.0f })
                    .sprite("assets/tileset.png:star")
                    .build();

  hurt_particle2 = Game::particle_builder()
                     .size(0.1f, 0.4f)
                     .life(30, 60)
                     .color(PALETTE_GREEN)
                     .velocity({ -0.6f, -0.6f }, { 0.6f, 0.1f })
                     .gravity(0.08f)
                     .sprite("assets/tileset.png:bit")
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
  const Vector2 position{ static_cast<float>(physics.x), static_cast<float>(physics.y) };

  if (!Game::on_screen(physics.mask.rect(physics.x, physics.y)))
  {
    physics.v.x = 0.0f;
    physics.v.y = 0.0f;
    return;
  }

  auto players = get_components<Player>();
  if (players.empty())
    return;

  const bool colliding_left  = physics.is_colliding_with_solid(-2, 0);
  const bool colliding_right = physics.is_colliding_with_solid(2, 0);
  const bool colliding_side  = colliding_left || colliding_right;
  const bool colliding_up    = physics.is_colliding_with_solid(0, -16);
  const bool colliding_down  = physics.is_colliding_with_solid(0, 8);

  const auto &player   = players.front();
  auto &player_physics = get_component<Physics>(player.entity).get();
  const Vector2 player_position{ static_cast<float>(player_physics.x), static_cast<float>(player_physics.y) };

  if (type == Type::Slime)
  {
    if (physics.is_standing())
    {
      int jump_prob = 1;
      if (fabs(physics.v.x) < 0.1f || colliding_side)
        jump_prob = 30;

      if (chance(jump_prob))
      {
        physics.v.y = -3.0f;
      }
    }

    if (fabs(physics.v.x) < 0.5f)
      target = player_position;

    if (target.x != 0 && target.y != 0)
    {
      if (target.x < physics.x)
        physics.v.x = lerp(physics.v.x, -0.5f, 0.1f);
      else if (target.x > physics.x)
        physics.v.x = lerp(physics.v.x, 0.5f, 0.1f);
    }
  }
  else if (type == Type::Bat)
  {
    if (colliding_side && !colliding_up)
    {
      target.x = position.x;
      target.y = position.y - physics.mask.height * 2;
    }
    else if (colliding_side && !colliding_down)
    {
      target.x = position.x;
      target.y = position.y + physics.mask.height * 2;
    }
    else
    {
      float vis_size{ 80.0f };
      if (hurted)
        vis_size *= 2.0f;
      const Rectangle visibility_rect{ position.x - vis_size / 2.0f, position.y, vis_size, vis_size };
#if defined(DEBUG) && defined(DEBUG_ENEMY)
      Game::defer_draw(entity,
                       [=]
                       {
                         DrawRectangleLines(static_cast<int>(visibility_rect.x),
                                            static_cast<int>(visibility_rect.y),
                                            static_cast<int>(visibility_rect.width),
                                            static_cast<int>(visibility_rect.height),
                                            PALETTE_YELLOW);
                       });
#endif
      if (CheckCollisionPointRec(player_position, visibility_rect))
      {
        const float player_dist = Vector2Distance(position, player_position);
        if (player_dist < 40 && player_dist > 16)
        {
          target.x = player_physics.x;
          target.y = player_physics.y;
        }
        else if (chance(50) || target.x == 0)
        {
          target.x = player_physics.x;
          target.y = player_physics.y - randf(10, 50);
        }
      }
      else
      {
        if (chance(10))
        {
          target.x = position.x + randf(-50, 50);
          if (!colliding_up)
            target.y = position.y - randf(5, 30);
          else if (!colliding_down)
            target.y = position.y + randf(5, 10);
        }
      }
    }

    if (target.x != 0 && target.y != 0)
    {
      if (target.x < physics.x)
        physics.v.x = lerp(physics.v.x, -1.0f, 0.1f);
      else if (target.x > physics.x)
        physics.v.x = lerp(physics.v.x, 1.0f, 0.1f);

      if (target.y < physics.y)
        physics.v.y = lerp(physics.v.y, -1.0f, 0.1f);
      else if (target.y > physics.y)
        physics.v.y = lerp(physics.v.y, 1.0f, 0.1f);
    }
  }
}

void Enemy::postupdate()
{
  auto &physics = get_component<Physics>(entity).get();

  dir_x = physics.v.x > 0 ? 1 : -1;

  auto &light    = get_component<Light>(entity).get();
  auto &renderer = get_component<SpriteRenderer>(entity).get();
  renderer.set_position(physics.x, physics.y);

  if (type == Type::Slime)
  {
    if (fabs(physics.v.x) < 0.1f)
      renderer.sprite_interpolated.animation_speed = 0;
    else
      renderer.sprite_interpolated.animation_speed = 1;
  }
  else if (type == Type::Bat)
  {
    renderer.sprite_interpolated.sprite.set_frame_durations({ fabs(physics.v.x) < 0.1f ? 200 : 100, 100 });
  }

  if (dir_x != 0)
    renderer.sprite_interpolated.sprite.scale.x = static_cast<float>(dir_x);

  auto &hurtable = get_component<Hurtable>(entity).get();
  if (hurtable.process())
  {
    hurted          = true;
    light.intensity = 1.0f;
    Game::add_particles(physics.x, physics.y, hurt_particle, 2);
    Game::add_particles(hurtable.hit_point_x, hurtable.hit_point_y, hurt_particle2, 20);
    Game::skip_ticks(1);

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
      physics.collidable                   = false;
      physics.solid                        = false;
      physics.do_update                    = false;

      Game::add_timer(entity, [&] { destroy_entity(entity); }, 30);
      Game::skip_ticks(1);
    }
  }

  light.x         = physics.x;
  light.y         = physics.y;
  light.size      = lerp(light.size, 1.0f, 0.1f);
  light.intensity = lerp(light.intensity, 0.0f, 0.2f);
}
