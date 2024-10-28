#include "level_loader.hpp"

#include <csignal>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "ldtk.hpp"

#if defined(DEBUG)
  #define ASSERT_RET(x, msg) \
    {                        \
      assert((x && msg));    \
    }

  #define ASSERT_RET_VAL(x, msg, ret) \
    {                                 \
      assert((x && msg));             \
    }
#else
  #define ASSERT_RET(x, msg)     \
    if (!(x))                    \
    {                            \
      fprintf(stderr, msg "\n"); \
      return;                    \
    }

  #define ASSERT_RET_VAL(x, msg, ret) \
    if (!(x))                         \
    {                                 \
      fprintf(stderr, msg "\n");      \
      return ret;                     \
    }
#endif

[[nodiscard]] static std::filesystem::path get_assets_path()
{
  return "assets";
}

[[nodiscard]] static std::filesystem::path get_level_project_path()
{
  const std::string file_name = "level.ldtk";
  return get_assets_path() / file_name;
}

[[nodiscard]] static std::filesystem::path get_tileset_path(const std::string &file_name)
{
  return get_assets_path() / file_name;
}

struct Cache
{
  ldtk::Ldtk ldtk_project;
  std::unordered_map<Level::TilesetId, Level::Tileset> tilesets;
};

static std::unique_ptr<Cache> cache;
static ldtk::Level null_level{};

[[nodiscard]] static Level::TilesetMap load_tilesets()
{
  Level::TilesetMap tilesets;

  for (const auto &tileset_def : cache->ldtk_project.defs.tilesets)
  {
    if (!tileset_def.rel_path.has_value())
      continue;

    Level::Tileset tileset;
    tileset.id   = tileset_def.uid;
    tileset.path = get_tileset_path(tileset_def.rel_path.value_or(""));

    for (const auto &enum_tag : tileset_def.enum_tags)
    {
      const auto &id       = enum_tag.enum_value_id;
      auto &enum_tiles_map = tileset.enum_tiles[id];
      std::transform(enum_tag.tile_ids.begin(),
                     enum_tag.tile_ids.end(),
                     std::inserter(enum_tiles_map, enum_tiles_map.begin()),
                     [](const auto &tile_id) { return tile_id; });
    }

    tilesets[tileset.id] = tileset;
  }

  return tilesets;
}

void LevelLoader::load_project()
{
  const std::filesystem::path path = get_level_project_path();
  ASSERT_RET(std::filesystem::exists(path), "Level file not found");

  unload_project();

  std::ifstream file(path);
  ASSERT_RET(file.is_open(), "Failed to open level file");

  if (!std::filesystem::exists(path) || !file.is_open())
  {
    fprintf(stderr, "Failed to open level file\n");
    return;
  }

  nlohmann::json json = nlohmann::json::parse(file);
  cache               = std::make_unique<Cache>();
  cache->ldtk_project = json.get<ldtk::Ldtk>();

  file.close();
  ASSERT_RET(!cache->ldtk_project.levels.empty(), "No levels found");

  cache->tilesets = load_tilesets();
}

void LevelLoader::unload_project()
{
  cache.reset();
}

[[nodiscard]] bool LevelLoader::is_project_loaded()
{
  return cache != nullptr && !cache->ldtk_project.levels.empty();
}

[[nodiscard]] static inline std::string get_level_identifier(const std::string &iid)
{
  ASSERT_RET_VAL(LevelLoader::is_project_loaded(), "Project not loaded", "");

  for (const auto &level : cache->ldtk_project.levels)
  {
    if (level.iid == iid)
      return level.identifier;
  }

  ASSERT_RET_VAL(false, "Level identifier not found", "");
  return {};
}

[[nodiscard]] static inline ldtk::Level &get_level(const std::string &name)
{
  ASSERT_RET_VAL(cache, "Project not loaded", null_level);

  auto level_it = std::find_if(cache->ldtk_project.levels.begin(),
                               cache->ldtk_project.levels.end(),
                               [&name](const ldtk::Level &level) { return level.identifier == name; });

  ASSERT_RET_VAL(level_it != cache->ldtk_project.levels.end(), "Level not found", null_level);

  return *level_it;
}

[[nodiscard]] static inline Level::Direction direction_from_string(const std::string &dir)
{
  if (dir == "n")
    return Level::Direction::North;
  if (dir == "e")
    return Level::Direction::East;
  if (dir == "s")
    return Level::Direction::South;
  if (dir == "w")
    return Level::Direction::West;
  if (dir == "ne")
    return Level::Direction::NorthEast;
  if (dir == "nw")
    return Level::Direction::NorthWest;
  if (dir == "se")
    return Level::Direction::SouthEast;
  if (dir == "sw")
    return Level::Direction::SouthWest;

  fprintf(stderr, "Invalid direction %s\n", dir.c_str());
  ASSERT_RET_VAL(false, "Invalid direction", Level::Direction::North);
  return Level::Direction::North;
}

[[nodiscard]] static Level::NeighbourMap load_neighbours(const std::vector<ldtk::NeighbourLevel> &ldtk_neighbours)
{
  Level::NeighbourMap neighbours;

  for (const auto &neighbor : ldtk_neighbours)
  {
    const auto &neighbor_identifier = get_level_identifier(neighbor.level_iid);
    const auto &direction           = direction_from_string(neighbor.dir);

    if (!neighbours.contains(direction))
      neighbours[direction] = neighbor_identifier;
    else
    {
      fprintf(stderr, "Duplicate neighbour %s\n", neighbor.dir.c_str());
      ASSERT_RET_VAL(false, "Duplicate neighbour", {});
    }
  }

  return neighbours;
}

[[nodiscard]] static inline int get_layer_depth(const ldtk::LayerInstance &layer)
{
  if (layer.identifier.starts_with("Front"))
    return -10;
  if (layer.identifier == "Default")
    return 0;
  else if (layer.identifier.starts_with("Back"))
    return 10;

  ASSERT_RET_VAL(false, "Invalid layer depth", 0);
  return 0;
}

[[nodiscard]] static Level::TilesVec load_tiles(const ldtk::LayerInstance &layer)
{
  ASSERT_RET_VAL(!layer.grid_tiles.empty(), "No grid tiles found", {});

  Level::TilesVec tiles;

  const auto &grid_size   = layer.grid_size;
  const auto &tileset_uid = layer.tileset_def_uid.value_or(0);

  for (const auto &tile : layer.grid_tiles)
  {
    ASSERT_RET_VAL(tile.px.size() >= 2, "Incorrect tile px size", tiles);
    ASSERT_RET_VAL(tile.src.size() >= 2, "Incorrect tile src size", tiles);

    Level::Tile t;
    t.tileset_id               = tileset_uid;
    t.id                       = tile.t;
    t.depth                    = get_layer_depth(layer);
    t.position.x               = tile.px[0];
    t.position.y               = tile.px[1];
    t.size.w                   = grid_size;
    t.size.h                   = grid_size;
    t.source_position.source_x = tile.src[0];
    t.source_position.source_y = tile.src[1];

    tiles.push_back(t);
  }

  return tiles;
}

[[nodiscard]] static inline Level::Tile load_tile(const ldtk::TilesetRectangle &rect)
{
  Level::Tile t;
  t.tileset_id               = rect.tileset_uid;
  t.source_position.source_x = rect.x;
  t.source_position.source_y = rect.y;
  t.size.w                   = rect.w;
  t.size.h                   = rect.h;

  return t;
}

[[nodiscard]] static inline Color color_from_hex(const std::string &hex)
{
  const auto r = std::stoi(hex.substr(1, 2), nullptr, 16);
  const auto g = std::stoi(hex.substr(3, 2), nullptr, 16);
  const auto b = std::stoi(hex.substr(5, 2), nullptr, 16);

  return Color{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), 255 };
}

[[nodiscard]] static Level::FieldMap load_fields(const std::vector<ldtk::FieldInstance> &field_instances)
{
  Level::FieldMap fields;

  for (const auto &ins : field_instances)
  {
    if (ins.value.is_null())
    {
      printf("Field %s is null\n", ins.identifier.c_str());
      continue;
    }

    if (ins.type == "Tile")
    {
      ASSERT_RET_VAL(ins.value.is_object(), "Tile field value is not an object", {});

      Level::Tile t          = load_tile(ins.value.get<ldtk::TilesetRectangle>());
      fields[ins.identifier] = t;
    }
    else if (ins.type == "String" || ins.type.starts_with("LocalEnum."))
    {
      ASSERT_RET_VAL(ins.value.is_string(), "String field value is not a string", {});
      fields[ins.identifier] = ins.value.get<std::string>();
    }
    else if (ins.type == "Color")
    {
      ASSERT_RET_VAL(ins.value.is_string(), "Color field value is not a string", {});
      fields[ins.identifier] = color_from_hex(ins.value.get<std::string>());
    }
    else if (ins.type == "Int")
    {
      ASSERT_RET_VAL(ins.value.is_number_integer(), "Int field value is not an integer", {});
      fields[ins.identifier] = ins.value.get<int>();
    }
    else if (ins.type == "Bool")
    {
      ASSERT_RET_VAL(ins.value.is_boolean(), "Bool field value is not a boolean", {});
      fields[ins.identifier] = ins.value.get<bool>();
    }
    else if (ins.type == "Float")
    {
      if (!ins.value.is_number_float() && ins.value.is_number())
        fields[ins.identifier] = static_cast<float>(ins.value.get<int>());
      else
      {
        ASSERT_RET_VAL(ins.value.is_number_float(), "Float field value is not a float", {});
        fields[ins.identifier] = ins.value.get<float>();
      }
    }
    else if (ins.type == "EntityRef")
    {
      printf("EntityRef field %s\n", ins.identifier.c_str());
      if (ins.value.is_null())
        continue;

      ldtk::ReferenceToAnEntityInstance ref = ins.value.get<ldtk::ReferenceToAnEntityInstance>();
      printf("EntityRef field %s\n", ref.entity_iid.c_str());

      fields[ins.identifier] = Level::EntityRef(ref.entity_iid);
    }
    else
    {
      fprintf(stderr, "Field %s, type %s unsupported\n", ins.identifier.c_str(), ins.type.c_str());
      ASSERT_RET_VAL(false, "Unsupported field type", fields);
    }
  }

  return fields;
}

[[nodiscard]] static Level::EntityMap load_entities(const ldtk::LayerInstance &layer)
{
  Level::EntityMap entities;

  for (const auto &ins : layer.entity_instances)
  {
    ASSERT_RET_VAL(ins.pivot.size() >= 2, "Incorrect pivot size", {});
    ASSERT_RET_VAL(ins.px.size() >= 2, "Incorrect px size", {});

    const auto &pivot_x = ins.pivot[0];
    const auto &pivot_y = ins.pivot[1];

    Level::Entity e;
    e.id         = ins.iid;
    e.name       = ins.identifier;
    e.position.x = ins.px[0] - pivot_x * ins.width;
    e.position.y = ins.px[1] - pivot_y * ins.height;
    e.size.w     = ins.width;
    e.size.h     = ins.height;

    if (ins.tile.has_value())
    {
      e.tile           = load_tile(ins.tile.value());
      e.tile->position = e.position;
    }

    e.fields               = load_fields(ins.field_instances);
    e.fields["Identifier"] = e.id;

    entities[e.id] = e;
  }

  return entities;
}

LevelLoader::LevelLoader(const std::string &name)
{
  load(name);
}

void LevelLoader::load(const std::string &name)
{
  if (!is_project_loaded())
    load_project();

  ASSERT_RET(is_project_loaded(), "Project not loaded");

  const ldtk::Level &level = get_level(name);
  this->iid                = level.iid;
  this->name               = level.identifier;
  printf("Loading level %s (%s)\n", this->name.c_str(), this->iid.c_str());
  this->world_x    = level.world_x;
  this->world_y    = level.world_y;
  this->width      = level.px_wid;
  this->height     = level.px_hei;
  this->neighbours = load_neighbours(level.neighbours);
  this->fields     = load_fields(level.field_instances);

  ASSERT_RET(cache, "Level cache not loaded");
  this->tilesets = cache->tilesets;

  this->tiles.clear();
  this->entities.clear();
  ASSERT_RET(level.layer_instances.has_value(), "No layer instances found");
  for (const auto &layer : level.layer_instances.value())
  {
    if (!layer.grid_tiles.empty())
    {
      ASSERT_RET(!this->tilesets.empty(), "No tilesets found");
      const auto &&layer_tiles = load_tiles(layer);
      this->tiles.insert(std::end(this->tiles), std::begin(layer_tiles), std::end(layer_tiles));
    }

    if (!layer.entity_instances.empty())
    {
      const auto &&layer_entities = load_entities(layer);
      this->entities.insert(std::begin(layer_entities), std::end(layer_entities));
    }
  }
}
