#pragma once

#include <cstddef>

#include <raylib.h>

struct GameLibrary
{
  using create_game_fn  = void (*)();
  using destroy_game_fn = void (*)();
  using draw_game_fn    = void (*)(double, RenderTexture &, RenderTexture &);
  using reload_game_fn  = void (*)();
  using unload_game_fn  = void (*)();
  using update_game_fn  = void (*)();

  create_game_fn create_game{ nullptr };
  destroy_game_fn destroy_game{ nullptr };
  draw_game_fn draw_game{ nullptr };
  reload_game_fn reload_game{ nullptr };
  unload_game_fn unload_game{ nullptr };
  update_game_fn update_game{ nullptr };

  bool has_library_changed();
  bool is_being_written();
  bool is_loaded();
  bool is_valid();
  bool load();
  bool unload();
  bool reload();

private:
  void *handle{ nullptr };
};
