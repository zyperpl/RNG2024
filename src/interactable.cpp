#include "interactable.hpp"

#include "game.hpp"
#include "level.hpp"
#include "physics.hpp"
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
