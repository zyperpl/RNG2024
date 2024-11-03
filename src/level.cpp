#include "level.hpp"

#include "block.hpp"
#include "level_loader.hpp"
#include "player.hpp"
#include "renderers.hpp"

#include "magic_enum.hpp"

namespace Level
{
LevelRegistry &LevelRegistry::get()
{
  static LevelRegistry instance;
  return instance;
}

Level::~Level()
{
  if (level_loader)
    delete level_loader;
}

void Level::load(const std::string &name)
{
  destroy_non_persistent_entities();
  auto &manager = Manager::get();
  manager.call_destroy();

  if (!level_loader)
    level_loader = new LevelLoader(name);
  else
    level_loader->load(name);

  for (const auto &tile : level_loader->tiles)
  {
    const auto &tileset = level_loader->tilesets[tile.tileset_id];

    if (tileset.enum_tiles.contains("Solid") && tileset.enum_tiles.at("Solid").contains(tile.id))
      add_entity(Block(tile));
  }

  create_entities(*level_loader);

  manager.call_init();
}

void Level::reload()
{
  if (level_loader)
    load(level_loader->name);
}

void Level::create_entities(const LevelLoader &level_loader)
{
  const auto &tiles = level_loader.tiles;

  auto tile_entity = create_entity();
  for (const auto &tile : tiles)
  {
    const auto &tileset_id = tile.tileset_id;

    auto position = tile.position;
    position.x += tile.size.w / 2;
    position.y += tile.size.h / 2;
    const auto &tileset = level_loader.tilesets.at(tileset_id);
    auto &tile_renderer =
      add_component(tile_entity, TileRenderer{ tileset.path, position, tile.size, tile.source_position }).get();
    tile_renderer.depth = tile.depth;
  }

  // create entities ids
  auto &entity_registry = LevelRegistry::get().entity_registry;
  std::map<std::string, ::Entity> entity_ids;
  for (const auto &[id, level_entity] : level_loader.entities)
  {
    if (entity_registry.contains(level_entity.name))
    {
      entity_ids[id] = create_entity();
      printf("Created entity %s (%lu)\n", id.c_str(), entity_ids[id]);
    }

    if (level_entity.name == "PlayerPosition")
    {
      if (reset_player_position)
      {
        if (auto players = get_components<Player>(); !players.empty())
        {
          auto &player  = players.front();
          auto &physics = get_component<Physics>(player.entity).get();
          physics.x     = level_entity.position.x;
          physics.y     = level_entity.position.y;

          physics.do_update = true;
          physics.collidable = true;

          reset_player_position = false;
        }
      }
    }
  }

  // resolve entity references
  for (auto &[id, level_entity] : level_loader.entities)
  {
    for (auto &[field_name, field] : level_entity.fields)
    {
      if (std::holds_alternative<EntityRef>(field))
      {
        auto &entity_ref = std::get<EntityRef>(field);
        if (entity_ids.contains(entity_ref.level_entity_id))
        {
          entity_ref.game_entity_id = entity_ids.at(entity_ref.level_entity_id);
          printf("Resolved entity reference %s (%lu)\n", entity_ref.level_entity_id.c_str(), entity_ref.game_entity_id);
        }
        else
        {
          fprintf(stderr, "Entity reference not found in the current level: %s\n", entity_ref.level_entity_id.c_str());
        }
      }
    }
  }

  // invoke entity component constructors
  for (const auto &[id, level_entity] : level_loader.entities)
  {
    if (entity_registry.contains(level_entity.name))
    {
      entity_registry[level_entity.name](level_entity, entity_ids.at(id));
    }
  }
}

int64_t Level::get_world_x() const
{
  assert(level_loader && "Level loader is not created");

  return level_loader->world_x;
}

int64_t Level::get_world_y() const
{
  assert(level_loader && "Level loader is not created");

  return level_loader->world_y;
}

int64_t Level::get_width() const
{
  assert(level_loader && "Level loader is not created");

  return level_loader->width;
}

int64_t Level::get_height() const
{
  assert(level_loader && "Level loader is not created");

  return level_loader->height;
}

std::string Level::get_name() const
{
  assert(level_loader && "Level loader is not created");

  return level_loader->name;
}

void Level::load_neighbour(Direction dir)
{
  assert(level_loader && "Level loader is not created");

  const auto &neighbours = level_loader->neighbours;
  if (!neighbours.contains(dir))
    return;

  const auto &neighbour = neighbours.at(dir);
  load(neighbour);
}

} // namespace Level
