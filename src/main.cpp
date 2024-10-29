#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>

#include <raylib.h>
#include <raymath.h>

#if defined(EMSCRIPTEN)
  #include <emscripten.h>
#endif

#include "input.hpp"
#include "lib.hpp"
#include "rl_utils.hpp"
#include "utils.hpp"

const int GAME_WIDTH{ 320 };
const int GAME_HEIGHT{ 180 };
const char *const GAME_TITLE{ "Game" };
const double INTERNAL_FPS{ 60.0 };
const Color BACKGROUND_COLOR{ 85, 65, 95, 255 };
const Color PALETTE_WHITE{ 220, 245, 255, 255 };

void *manager_memory{ nullptr };
void *allocate_manager(size_t alignment, size_t size)
{
  if (!manager_memory)
  {
    manager_memory = std::aligned_alloc(alignment, size * 2);
    assert(manager_memory);
    std::align(alignment, size, manager_memory, size);
    std::memset(manager_memory, 0, size);
  }

  printf("Manager memory: %p\n", manager_memory);
  printf("Manager size: %zu\n", size);

  return manager_memory;
}

void *game_memory{ nullptr };
void *allocate_game(size_t alignment, size_t size)
{
  if (!game_memory)
  {
    game_memory = std::aligned_alloc(alignment, size * 2);
    assert(game_memory);
    std::align(alignment, size, game_memory, size);
    std::memset(game_memory, 0, size);
  }

  printf("Game memory: %p\n", game_memory);
  printf("Game size: %zu\n", size);

  return game_memory;
}

[[nodiscard]] static double get_time()
{
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<double>(now.time_since_epoch()).count();
}

[[nodiscard]] static Rectangle get_render_destination()
{
  const auto screen_width       = static_cast<float>(GetScreenWidth());
  const auto screen_height      = static_cast<float>(GetScreenHeight());
  const auto game_width         = static_cast<float>(GAME_WIDTH);
  const auto game_height        = static_cast<float>(GAME_HEIGHT);
  const auto scale              = std::min(screen_width / game_width, screen_height / game_height);
  const auto game_screen_width  = game_width * scale;
  const auto game_screen_height = game_height * scale;
  const auto game_screen_x      = (screen_width - game_screen_width) / 2.0f;
  const auto game_screen_y      = (screen_height - game_screen_height) / 2.0f;
  return Rectangle{ game_screen_x, game_screen_y, game_screen_width, game_screen_height };
}

struct Engine
{
  NRL::RenderTexture game_render_texture{ GAME_WIDTH, GAME_HEIGHT, NRL::RenderTexture::Smooth::No };
  NRL::RenderTexture interface_render_texture{ GAME_WIDTH, GAME_HEIGHT, NRL::RenderTexture::Smooth::Yes };

  bool game_created = false;

  uint64_t frame                  = 0;
  uint64_t step                   = 0;
  uint64_t library_reloads        = 0;
  double library_last_reload_time = get_time();
  double fps                      = 0.0;

  Engine()
  {
    game_render_texture.flip = true;
  }
};

std::unique_ptr<Engine> engine;

struct LoopInfo
{
  double accumulator{ 0.0 };
  double target_dt{ 1.0 / INTERNAL_FPS };
  double previous_time{ 0.0 };

  [[nodiscard]] double frame_progress() const
  {
    return std::clamp(accumulator / target_dt, 0.0, 1.0);
  }
};

LoopInfo loop;
GameLibrary game_library;

auto update_draw_frame()
{
  assert(engine && "Engine is not created");

  auto &input = INPUT;

  if (game_library.is_loaded() && game_library.is_valid())
  {
    if (!engine->game_created)
    {
      game_library.create_game();
      engine->game_created = true;
    }
    input.update_discrete();

    const auto current_time = get_time();
    loop.accumulator += current_time - loop.previous_time;
    loop.accumulator = std::clamp(loop.accumulator, 0.0, 0.3);
    if (engine->step % 10 == 0)
      engine->fps = 1.0 / (current_time - loop.previous_time);
    loop.previous_time = current_time;

    int current_frame_steps = 0;
    while (loop.accumulator >= loop.target_dt && current_frame_steps < 10)
    {
      game_library.update_game();
      input.update_continuous();

      loop.accumulator -= loop.target_dt;
      engine->step += 1;
      current_frame_steps += 1;
    }

    const auto render_destination = get_render_destination();

    engine->interface_render_texture.resize(render_destination.width, render_destination.height);

    game_library.draw_game(
      loop.frame_progress(), engine->game_render_texture.value, engine->interface_render_texture.value);
    engine->frame += 1;

    BeginDrawing();
    {
      ClearBackground(BACKGROUND_COLOR);

      engine->game_render_texture.draw(render_destination);
      engine->interface_render_texture.draw(render_destination);

      const auto DEBUG_KEY = KEY_F1;
      int text_y           = 0;
      int font_size        = 10;

      const auto reload_seconds_ago = current_time - engine->library_last_reload_time;
      const bool reload_recently    = reload_seconds_ago < 1.1;

#if defined(DEBUG)
      if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4))
      {
        fprintf(stderr, "Forcing exit\n");
        printf("Forcing exit\n");
        fflush(stdout);
        exit(0);
      }
      DrawText(TextFormat("FPS: %3.2f", engine->fps), 0, text_y, font_size, PALETTE_WHITE);
#endif

      if (IsKeyDown(DEBUG_KEY) || reload_recently)
      {
        DrawText(TextFormat("FPS: %3.2f", engine->fps), 0, text_y, font_size, PALETTE_WHITE);
        text_y += font_size;
        DrawText(TextFormat("Reloads: %8lu (%3.1fs ago)", engine->library_reloads, reload_seconds_ago),
                 0,
                 text_y,
                 font_size,
                 reload_recently ? RED : PALETTE_WHITE);
        text_y += font_size;
      }
    }
    EndDrawing();

    input.update_game_mouse(render_destination, engine->game_render_texture.rect());
    input.update_interface_mouse(render_destination, engine->interface_render_texture.rect());
  }
  else
  {
    BeginDrawing();
    {
      ClearBackground(BACKGROUND_COLOR);
      DrawText("Game library is not loaded", 0, 0, 10, PALETTE_WHITE);
    }
    EndDrawing();
  }

  const bool force_reload = IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R);
  if (game_library.has_library_changed() || !game_library.is_loaded() || force_reload)
  {
    printf("Starting reload process\n");
    if (game_library.is_loaded() && game_library.unload_game)
      game_library.unload_game();
    printf("Game components registry unloaded\n");
    if (game_library.reload())
    {
      assert(game_library.reload_game);
      game_library.reload_game();
      engine->library_last_reload_time = get_time();
      engine->library_reloads += 1;
    }
  }
}

#if defined(DEBUG)
  #include "gen.hpp"
#endif

auto main() -> int
{
#if defined(DEBUG)
  //generate_entity("Bullet", "bullet", { .is_level_entity = false, .has_particles = true, .has_sounds = true, .has_physics = true, .has_sprite_renderer = true, .has_update = true, .has_postupdate = true, .has_collision = true });
#endif

  InitWindow(GAME_WIDTH * 4, GAME_HEIGHT * 4, GAME_TITLE);

  const constexpr int AUDIO_BUFFER_SIZE = (4096 * 12);
  SetAudioStreamBufferSizeDefault(AUDIO_BUFFER_SIZE);
  InitAudioDevice();

  game_library.load();

  [[maybe_unused]] const auto monitor_refresh_rate = GetMonitorRefreshRate(0);
  SetTargetFPS(std::max(monitor_refresh_rate, 120));
  SetTargetFPS(0);

  engine             = std::make_unique<Engine>();
  loop.previous_time = get_time();

#if defined(EMSCRIPTEN)
  emscripten_set_main_loop(update_draw_frame, 0, 1);
#else
  while (!WindowShouldClose())
  {
    update_draw_frame();
  }

  game_library.destroy_game();
  game_library.unload();

  std::free(manager_memory);

  CloseAudioDevice();
  CloseWindow();

#endif
  printf("Done.\n");
}
