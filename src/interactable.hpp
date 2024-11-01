#pragma once

#include <string>
#include <variant>

#include "component.hpp"
#include "game.hpp"
#include "level_definitions.hpp"
#include "manager.hpp"

#include <raylib.h>

struct Interactable
{
  struct ActionMessage
  {
    std::string message;
    Color color{ PALETTE_WHITE };
  };

  struct ActionLevelStore
  {
    std::string entity_level_id;
    std::string key;
    Level::Field field;
  };

  using Action = std::variant<ActionMessage, ActionLevelStore>;

  COMPONENT(Interactable);

  template<typename... Action>
  [[nodiscard]] inline constexpr Interactable(Action... action)
    : actions{ action... }
  {
  }

  [[nodiscard]] bool can_interact(Rectangle other_rect) const;
  void interact();

  inline void enable()
  {
    enabled = true;
  }

  inline void disable()
  {
    enabled = false;
  }

  [[nodiscard]] inline bool is_used() const
  {
    return interacted;
  }

  [[nodiscard]] inline bool is_enabled() const
  {
    return enabled;
  }

  template<typename... Action>
  inline void add(Action... action)
  {
    actions.emplace_back(action...);
  }

private:
  bool enabled{ true };
  bool interacted{ false };
  std::vector<Action> actions;
};

EXTERN_COMPONENT_TEMPLATE(Interactable);
