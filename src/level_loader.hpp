#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "level_definitions.hpp"

struct LevelLoader
{
  LevelLoader(const Level::LevelName &);
  void load(const Level::LevelName &);

  bool loaded{ false };
  int64_t world_x{ 0 };
  int64_t world_y{ 0 };
  int64_t width{ 0 };
  int64_t height{ 0 };
  std::string iid;
  std::string name;

  Level::TilesetMap tilesets;
  Level::TilesVec tiles;
  mutable Level::EntityMap entities;
  Level::NeighbourMap neighbours;
  Level::FieldMap fields;

  static bool is_project_loaded();

private:
  static void load_project();
  static void unload_project();
};
