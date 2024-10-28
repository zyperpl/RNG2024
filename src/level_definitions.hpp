#pragma once

#include <cassert>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <raylib.h>

#include "component.hpp"

struct TilePosition
{
  int32_t x{ 0 };
  int32_t y{ 0 };
};

struct TileSize
{
  int32_t w{ 0 };
  int32_t h{ 0 };
};

struct TileSourcePosition
{
  int32_t source_x{ 0 };
  int32_t source_y{ 0 };
};

namespace Level
{
struct Tile;
struct Entity;
struct EntityRef;

using TilesetId = int64_t;
using TileId    = int64_t;
using TileIds   = std::set<TileId>;

using Field    = std::variant<Tile, Color, std::string, int, float, bool, EntityRef>;
using FieldMap = std::unordered_map<std::string, Field>;

struct Tileset
{
  TilesetId id;
  std::string path;
  std::map<std::string, TileIds> enum_tiles;
};

struct Tile
{
  TilesetId tileset_id;
  TileId id;
  TilePosition position;
  TileSourcePosition source_position;
  TileSize size;

  int depth{ 0 };
};

struct Entity
{
  LevelEntityId id;
  std::string name;
  TilePosition position;
  TileSize size;

  std::optional<Tile> tile;

  FieldMap fields;
};

struct EntityRef
{
  ::Entity game_entity_id{ INVALID_ENTITY };
  ::LevelEntityId level_entity_id{ "" };

  EntityRef() = default;

  EntityRef(const std::string &level_entity_id)
    : level_entity_id{ level_entity_id }
  {
  }
};

using LevelName = std::string;
enum Direction
{
  North,
  East,
  South,
  West,
  NorthWest,
  NorthEast,
  SouthWest,
  SouthEast
};
using TilesetMap   = std::unordered_map<Level::TilesetId, Level::Tileset>;
using TilesVec     = std::vector<Level::Tile>;
using EntityMap    = std::unordered_map<LevelEntityId, Level::Entity>;
using NeighbourMap = std::unordered_map<Direction, LevelName>;

template<typename T>
[[nodiscard]] std::optional<T> get_field(const FieldMap &fields, const std::string &name)
{
  if constexpr (std::is_same_v<T, ::Entity>)
  {
    if (auto field = get_field<EntityRef>(fields, name))
    {
      fprintf(stderr, "Field '%s' is not an entity reference\n", name.c_str());
      assert(field->game_entity_id > INVALID_ENTITY && "Invalid field entity id");
      return field->game_entity_id;
    }
  }
  else
  {
    if (fields.contains(name))
    {
      auto &field = fields.at(name);
      if (auto value = std::get_if<T>(&field))
        return *value;

      fprintf(stderr, "Field type mismatch for field '%s'\n", name.c_str());
      assert(false && "Field type mismatch");
    }
  }
  return std::nullopt;
}

template<typename T>
inline void read_field(const FieldMap &fields, const std::string &name, T &value)
{
  if (auto field = get_field<T>(fields, name))
    value = *field;
  else
  {
    fprintf(stderr, "Field '%s' not found\n", name.c_str());
    assert(false && "Field not found");
  }
}

template<typename T>
inline void try_read_field(const FieldMap &fields, const std::string &name, T &value)
{
  if (auto field = get_field<T>(fields, name))
    value = *field;
}

} // namespace Level
