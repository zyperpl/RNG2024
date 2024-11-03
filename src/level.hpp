#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "level_definitions.hpp"
#include "manager.hpp"
#include "sprite.hpp"

struct LevelLoader;

namespace Level
{
using EntityDef = Entity;

struct LevelRegistry
{
  void register_entity(std::string name, const std::function<void(const EntityDef &, ::Entity)> &callback)
  {
    entity_registry[name] = callback;
  }

  std::unordered_map<std::string, std::function<void(const EntityDef &, ::Entity)>> entity_registry;

  static LevelRegistry &get();
};

struct Level
{
  ~Level();
  void reload();
  void load(const std::string &name);
  void create_entities(const LevelLoader &);
  void load_neighbour(Direction);

  [[nodiscard]] int64_t get_world_x() const;
  [[nodiscard]] int64_t get_world_y() const;
  [[nodiscard]] int64_t get_width() const;
  [[nodiscard]] int64_t get_height() const;
  [[nodiscard]] std::string get_name() const;

  static inline std::unordered_map<LevelEntityId, std::unordered_map<std::string, Field>> entity_fields;
  static void store(const LevelEntityId &entity_id, const std::string &name, const Field &field)
  {
    printf("Storing field %s for entity %s\n", name.c_str(), entity_id.c_str());
    entity_fields[entity_id][name] = field;
  }

  static void load(const LevelEntityId &entity_id, const std::string &name, Field &field)
  {
    if (!entity_fields.contains(entity_id) || !entity_fields[entity_id].contains(name))
      return;

    field = entity_fields[entity_id][name];
  }

  bool reset_player_position { true };

private:
  LevelLoader *level_loader{ nullptr };
};

} // namespace Level

template<typename C>
struct LevelEntity
{
  LevelEntity()
  {
    printf("Registering level entity %s\n", C::name());
    Level::LevelRegistry::get().register_entity(C::name(), construct);
  }

  static void construct(const Level::Entity &entity, ::Entity game_entity_id)
  {
    if constexpr (std::is_constructible_v<C, Level::Entity>)
      add_component<C>(game_entity_id, C{ entity });
    else if constexpr (std::is_constructible_v<C, TilePosition>)
      add_component<C>(game_entity_id, C{ entity.position });
    else if constexpr (std::is_constructible_v<C, TilePosition, Level::FieldMap>)
      add_component<C>(game_entity_id, C{ entity.position, entity.fields });
    else if constexpr (std::is_constructible_v<C, Level::Tile>)
    {
      assert(entity.tile.has_value() && "Entity has no tile");
      if (!entity.tile.has_value())
      {
        fprintf(stderr, "Entity %s has no tile\n", entity.name.c_str());
        return;
      }
      add_component<C>(game_entity_id, C{ entity.tile.value() });
    }
    else
      static_assert(false, "No constructor found for component");
  }
};

#define REGISTER_LEVEL_ENTITY(C) static inline LevelEntity<C> level_entity_##C;
