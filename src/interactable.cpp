#include "interactable.hpp"

#include "game.hpp"
#include "hurtable.hpp"
#include "level.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Interactable);
COMPONENT_TEMPLATE(Interactable);

bool Interactable::can_interact(Rectangle other_rect) const
{
  if (!enabled)
    return false;

  auto physics_ref = get_component<Physics>(entity);
  if (physics_ref)
  {
    const auto &physics = physics_ref.get();
    return CheckCollisionRecs(physics.mask.rect(physics.x, physics.y), other_rect);
  }

  return false;
}

void execute_action(const Interactable::Action &action)
{
  if (auto action_msg = std::get_if<Interactable::ActionMessage>(&action))
  {
    Game::queue_message(action_msg->message, action_msg->color);
  }
  else if (auto level_store = std::get_if<Interactable::ActionLevelStore>(&action))
  {
    Level::Level::store(level_store->entity_level_id, level_store->key, level_store->field);
  }
  else if (auto level_store = std::get_if<Interactable::ActionEndLevel>(&action))
  {
    Game::end_level();
  }
  else if (auto pickup = std::get_if<Interactable::HealthPickup>(&action))
  {
    auto players = get_components<Player>();
    if (players.empty())
      return;

    auto &player      = players.front();
    auto hurtable_ref = get_component<Hurtable>(player.entity);
    if (!hurtable_ref)
      return;

    hurtable_ref.get().add_health(pickup->value);
  }
  else
  {
    assert(false && "Unknown interactable action type");
  }
}

void Interactable::interact()
{
  if (!enabled)
    return;

  interacted = true;

  for (const auto &action : actions)
  {
    execute_action(action);
  }
}

Vector2 Interactable::get_position() const
{
  auto physics_ref = get_component<Physics>(entity);
  if (physics_ref)
  {
    const auto &physics = physics_ref.get();
    return Vector2{ static_cast<float>(physics.x + physics.mask.width / 2), static_cast<float>(physics.y) };
  }

  return Vector2{ 0, 0 };
}
