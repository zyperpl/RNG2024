#pragma once

#include <iostream>
#include <string>

struct GenerateEntityParams
{
  bool is_level_entity{ false };
  bool has_init{ true };
  bool has_physics{ false };
  bool has_sprite_renderer{ false };
  bool has_tile_renderer{ false };
  bool has_particles{ false };
  bool has_sounds{ false };
  bool has_preupdate{ false };
  bool has_update{ false };
  bool has_postupdate{ false };
  bool has_postupdate_update_position{ false };
  bool has_render{ false };
  bool has_destroyed{ false };
  bool has_collision{ false };
};

void generate_entity(const std::string &component_name, const std::string &file_name, GenerateEntityParams params);
