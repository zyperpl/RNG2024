#include "physics.hpp"

#include "manager.hpp"

#include <algorithm>
#include <cmath>

REGISTER_COMPONENT(Physics);
COMPONENT_TEMPLATE(Physics);

// TODO: Implement DEBUG_PROFILE_MASK_CHECK
#define DEBUG_PROFILE_MASK_CHECK(name, mask, offset_x, offset_y)

void for_physics_components(std::function<void(Physics &)> &&func)
{
  auto physics_components = get_components<Physics>();
  for (auto &component : physics_components)
  {
    func(component);
  }
}

void Physics::update()
{
  bool ignore_physics = solid && !collidable && !movable;

  if (!ignore_physics)
  {
    update_previous();

    if (do_update)
    {
      check_collisions();
      update_movement();
      update_physics();
    }
  }
}

void Physics::update_previous()
{
  x_previous = x;
  y_previous = y;

  v_previous = v;
}

void Physics::check_collisions()
{
  if (!movable && !collidable)
    return;

  auto &manager = Manager::get();

  auto func = [this, &manager](Physics &other) -> void
  {
    if (&other == this)
      return;

    if (other.solid && solid)
      return;

    if (!other.movable && !movable)
      return;

    if (!other.collidable)
      return;

    auto vx = static_cast<int>(roundf(v.x + v_rem.x));
    auto vy = static_cast<int>(roundf(v.y + v_rem.y));
    if (is_colliding(other, vx, vy))
    {
      for (const auto &component_id : manager.collision_components)
      {
        const auto &container = manager.component_containers[component_id];
        container.collision(entity, other.entity);
      }
    }
  };

  for_physics_components(func);

  const auto vx = static_cast<int>(floorf(v.x));
  const auto vy = static_cast<int>(floorf(v.y));

  if (vx == 0 && vy == 0)
    return;

  auto hcolliding = is_colliding_with_solid(vx, 0.0f);
  if (hcolliding)
  {
    move_xy(vx, 0.0f);
  }

  auto vcolliding = is_colliding_with_solid(0.0f, vy);
  if (vcolliding)
  {
    move_xy(0.0f, vy);
  }

  if (hcolliding)
  {
    v.x     = 0;
    v_rem.x = 0;
  }

  if (vcolliding)
  {
    v.y     = 0;
    v_rem.y = 0;
  }
}

void Physics::update_physics()
{
  v.y += gravity;
  v.y = std::clamp(v.y, -32.0f, 32.0f);
}

void Physics::update_movement()
{
  move_xy(v.x, v.y);
}

bool Physics::is_colliding_with_any(int offset_x, int offset_y) const
{
  DEBUG_PROFILE_MASK_CHECK("any", mask, x + offset_x, y + offset_y);

  bool found = false;
  auto check = [this, &found, offset_x, offset_y](Physics &other)
  {
    if (&other == this)
      return false;

    if (!other.collidable)
      return false;

    if (is_colliding(other, offset_x, offset_y))
    {
      found = true;
      return true;
    }

    return false;
  };

  for_physics_components(check);

  return found;
}

bool Physics::is_colliding_with_nonsolid(int offset_x, int offset_y) const
{
  DEBUG_PROFILE_MASK_CHECK("nonsolid", mask, x + offset_x, y + offset_y);

  bool found = false;
  auto func  = [this, &found, offset_x, offset_y](Physics &other)
  {
    if (&other == this)
      return false;

    if (!other.collidable)
      return false;

    if (other.solid)
      return false;

    if (is_colliding(other, offset_x, offset_y))
    {
      found = true;
      return true;
    }

    return false;
  };

  for_physics_components(func);

  return found;
}

bool Physics::is_colliding_with_solid(int offset_x, int offset_y) const
{
  DEBUG_PROFILE_MASK_CHECK("solid", mask, x + offset_x, y + offset_y);

  bool found = false;
  auto func  = [this, &found, offset_x, offset_y](Physics &other)
  {
    if (&other == this)
      return false;

    if (!other.collidable)
      return false;

    if (!other.solid)
      return false;

    if (other.oneway && mask.bottom(y) > other.mask.top(other.y))
      return false;

    if (is_colliding(other, offset_x, offset_y))
    {
      found = true;
      return true;
    }

    return false;
  };

  for_physics_components(func);

  return found;
}

bool Physics::is_standing() const
{
  return v.y + v_rem.y >= 0.0f && is_colliding_with_solid(0, 1);
}

bool Physics::mask_free(Mask mask, int offset_x, int offset_y)
{
  DEBUG_PROFILE_MASK_CHECK("mask_free", mask, x + offset_x, y + offset_y);

  const Mask previous_mask = this->mask;
  this->mask               = mask;
  const bool free          = (!solid && !is_colliding_with_solid(offset_x, offset_y)) ||
                    (solid && !is_colliding_with_nonsolid(offset_x, offset_y));
  this->mask = previous_mask;

  return free;
}

EntitySet Physics::get_colliding_solid(int offset_x, int offset_y) const
{
  DEBUG_PROFILE_MASK_CHECK("solid", mask, x + offset_x, y + offset_y);

  EntitySet colliding_entities;

  auto func = [this, &colliding_entities, offset_x, offset_y](Physics &other)
  {
    if (&other == this)
      return;

    if (!other.collidable)
      return;

    if (!other.solid)
      return;

    if (other.oneway && mask.bottom(y) > other.mask.top(other.y))
      return;

    if (is_colliding(other, offset_x, offset_y))
    {
      colliding_entities.insert(other.entity);
    }
  };

  for_physics_components(func);

  return colliding_entities;
}

EntitySet Physics::get_colliding_nonsolid(int offset_x, int offset_y) const
{
  DEBUG_PROFILE_MASK_CHECK("nonsolid", mask, x + offset_x, y + offset_y);

  EntitySet colliding_entities;

  auto func = [this, &colliding_entities, offset_x, offset_y](Physics &other)
  {
    if (&other == this)
      return;

    if (!other.collidable)
      return;

    if (other.solid)
      return;

    if (other.oneway && mask.bottom(y) > other.mask.top(other.y))
      return;

    if (is_colliding(other, offset_x, offset_y))
      colliding_entities.insert(other.entity);
  };

  for_physics_components(func);

  return colliding_entities;
}

EntitySet Physics::get_colliding_any(int offset_x, int offset_y) const
{
  DEBUG_PROFILE_MASK_CHECK("any", mask, x + offset_x, y + offset_y);

  EntitySet colliding_entities;

  auto func = [this, &colliding_entities, offset_x, offset_y](Physics &other)
  {
    if (&other == this)
      return;

    if (!other.collidable)
      return;

    if (is_colliding(other, offset_x, offset_y))
      colliding_entities.insert(other.entity);
  };

  for_physics_components(func);

  return colliding_entities;
}

void Physics::move_xy(float vec_x, float vec_y)
{
  v_rem.x += vec_x;
  v_rem.y += vec_y;

  int move_x = static_cast<int>(roundf(v_rem.x));
  int move_y = static_cast<int>(roundf(v_rem.y));

  const int X_MOVE_LIMIT = 16;
  const int Y_MOVE_LIMIT = 24;
  move_x                 = std::clamp(move_x, -X_MOVE_LIMIT, X_MOVE_LIMIT);
  move_y                 = std::clamp(move_y, -Y_MOVE_LIMIT, Y_MOVE_LIMIT);

  if (move_x == 0 && move_y == 0)
    return;

  std::set<Physics *> riding_components;
  if (solid)
    riding_components = get_riding();

  if (move_x != 0)
  {
    const int sign = move_x > 0 ? 1 : -1;

    while (move_x != 0)
    {
      v_rem.x -= sign;

      if (!solid && is_colliding_with_solid(sign, 0))
      {
        v_rem.x = 0;
        v.x     = 0.0f;
        break;
      }

      x += sign;
      move_x -= sign;

      if (solid)
      {
        for (auto &pushable : get_pushable())
        {
          if (riding_components.contains(pushable))
            continue;

          auto previous_v     = pushable->v;
          auto previous_v_rem = pushable->v_rem;
          pushable->move_xy(sign, 0);
          pushable->v     = previous_v;
          pushable->v_rem = previous_v_rem;

          if (is_colliding(*pushable, 0, 0))
          {
            // TODO: Collision (collider squished) callback when pushing
          }
        }

        for (auto &physics : riding_components)
        {
          physics->move_xy(sign, 0);

          if (is_colliding(*physics, 0, 0))
          {
            // TODO: Collision (collider squished) callback when riding
          }
        }
      }
    }
  }

  if (move_y != 0)
  {
    const int sign = move_y > 0 ? 1 : -1;

    while (move_y != 0)
    {
      v_rem.y -= sign;

      if (!solid && is_colliding_with_solid(0, sign))
      {
        v_rem.y = 0;
        v.y     = 0.0f;
        break;
      }

      y += sign;
      move_y -= sign;

      if (solid)
      {
        for (auto &pushable : get_pushable())
        {
          if (riding_components.contains(pushable))
            continue;

          auto previous_v     = pushable->v;
          auto previous_v_rem = pushable->v_rem;
          pushable->move_xy(0, sign);
          pushable->v     = previous_v;
          pushable->v_rem = previous_v_rem;

          if (is_colliding(*pushable, 0, 0))
          {
            // TODO: Collision (collider squished) callback when pushing
          }
        }

        for (auto &physics : riding_components)
        {
          physics->move_xy(0, sign);

          if (is_colliding(*physics, 0, 0))
          {
            // TODO: Collision (collider squished) callback when riding
          }
        }
      }
    }
  }
}

std::set<Physics *> Physics::get_riding() const
{
  DEBUG_PROFILE_MASK_CHECK("riding", mask, x + 0, y + -1);

  std::set<Physics *> riding_components;

  const auto func = [this, &riding_components](Physics &other)
  {
    if (&other == this)
      return false;

    if (!other.movable)
      return false;

    if (!other.collidable)
      return false;

    if (mask.top(y) < other.mask.bottom(other.y))
      return false;

    if (other.v.y >= 0.0f && is_colliding(other, 0, -1))
    {
      riding_components.insert(&other);
      return true;
    }

    return false;
  };

  for_physics_components(func);

  return riding_components;
}

std::set<Physics *> Physics::get_pushable() const
{
  DEBUG_PROFILE_MASK_CHECK("pushable", mask, x + 0, y + 0);

  std::set<Physics *> colliding_entities;

  const auto func = [this, &colliding_entities](Physics &other)
  {
    if (&other == this)
      return false;

    if (other.solid)
      return false;

    if (!other.movable)
      return false;

    if (!other.collidable)
      return false;

    if (oneway && mask.bottom(y) > other.mask.top(other.y))
      return false;

    if (!is_colliding(other, 0, 0))
      return false;

    colliding_entities.insert(&other);
    return true;
  };

  for_physics_components(func);

  return colliding_entities;
}
