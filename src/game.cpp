#include "game.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>

#include <raylib.h>

#include "component.hpp"
#include "input.hpp"
#include "level.hpp"
#include "light.hpp"
#include "manager.hpp"
#include "player.hpp"
#include "renderers.hpp"
#include "utils.hpp"

static Game *game{ nullptr };

extern "C"
{
  void G_create_game()
  {
    printf("Creating game\n");
    assert(!game);

    if (!game)
    {
      void *game_ptr = allocate_game(std::alignment_of_v<Game>, sizeof(Game));
      if (*static_cast<int *>(game_ptr) == 0)
      {
        game = new (game_ptr) Game();
        printf("Game created at %p\n", (void *)game);
      }
      else
        game = static_cast<Game *>(game_ptr);
    }
    add_entity(Player());

    game->particle_system = add_component(create_entity(), ParticleSystem(-2));
    {
      auto &sprite =
        game->particle_system.get().get_sprite(game->particle_system.get().sprite_id("assets/tileset.png:dust"));
      sprite.source_offset.x = 8;
      sprite.source_offset.y = 216;
      sprite.set_frame_width(8);
      sprite.set_frame_height(8);
      sprite.set_frame_count(1);
    }
    {
      auto &sprite =
        game->particle_system.get().get_sprite(game->particle_system.get().sprite_id("assets/tileset.png:star"));
      sprite.source_offset.x = 8 + 8;
      sprite.source_offset.y = 216;
      sprite.set_frame_width(8);
      sprite.set_frame_height(8);
      sprite.set_frame_count(1);
    }

    auto &manager = Manager::get();
    manager.call_init();
    game->level.load("Level_0");
  }

  void G_reload_game()
  {
    CloseAudioDevice();
    InitAudioDevice();
    printf("Reloading game\n");

    [[maybe_unused]] auto &manager = Manager::get();
    game                           = static_cast<Game *>(allocate_game(std::alignment_of_v<Game>, sizeof(Game)));
    game->particle_system          = ComponentReference<ParticleSystem>(0);
    game->level.reload();
  }

  void G_unload_game()
  {
    printf("Unloading game\n");
    for (auto &timer : game->timers)
      timer.callback();
    game->timers.clear();

    auto &manager = Manager::get();
    manager.unregister_all();
  }

  void G_destroy_game()
  {
    printf("Destroying game\n");

    G_unload_game();
    if (game)
      delete game;
    game = nullptr;

    Manager::get().destroy();
  }

  void G_draw_game(double frame_progress, RenderTexture &game_render_texture, RenderTexture &interface_render_texture)
  {
    assert(game && "Game is not created");
    auto &game = Game::get();

    if (IsMusicStreamPlaying(game.music))
      UpdateMusicStream(game.music);

    auto &manager = Manager::get();

    const size_t MAX_LIGHTS = 32;
    static Vector2 light_pos[MAX_LIGHTS];
    static float light_size[MAX_LIGHTS];
    static float light_intensity[MAX_LIGHTS];
    for (size_t i = 0; i < MAX_LIGHTS; i++)
    {
      light_pos[i]       = Vector2{ -990.0f, -990.0f };
      light_size[i]      = 0.0f;
      light_intensity[i] = 0.0f;
      light_size[i]      = 1.0f;
    }

    size_t light_idx                  = 0;
    auto &camera                      = game.camera;
    light_pos[light_idx].x            = Input::get().game_mouse().x - camera.offset.x + camera.target.x;
    light_pos[light_idx].y            = Input::get().game_mouse().y - camera.offset.y + camera.target.y;
    static float mouse_light_strength = 0.0f;
    if (Input::get().mouse_wheel_down)
      mouse_light_strength -= 0.2f;
    if (Input::get().mouse_wheel_up)
      mouse_light_strength += 0.2f;
    if (mouse_light_strength < 0.0f)
      mouse_light_strength = 0.0f;
    light_intensity[light_idx] = mouse_light_strength;
    light_idx += 1;

    for (const auto &light : get_components<Light>())
    {
      if (light.x + 20 + light.intensity * 16 + light.size * 12 < camera.target.x - camera.offset.x ||
          light.x - 20 - light.intensity * 16 - light.size * 12 > camera.target.x + camera.offset.x ||
          light.y + 20 + light.intensity * 8 + light.size * 8 < camera.target.y - camera.offset.y ||
          light.y - 20 - light.intensity * 8 - light.size * 8 > camera.target.y + camera.offset.y)
        continue;

      if (light.size < 0.1f)
        continue;

      if (light.intensity < 0.1f)
        continue;

      light_pos[light_idx].x     = light.x;
      light_pos[light_idx].y     = light.y;
      light_size[light_idx]      = light.size;
      light_intensity[light_idx] = light.intensity;
      light_idx += 1;

      if (light_idx >= MAX_LIGHTS)
      {
        printf("Reached maximum number of lights (%zu)\n", MAX_LIGHTS);
        break;
      }
    }

    game.frames += 1;

    const double SCALE = 1.0;
    game.render_texture.resize(game_render_texture.texture.width / SCALE, game_render_texture.texture.height / SCALE);
    game.dither_fx.resize(game_render_texture.texture.width, game_render_texture.texture.height);
    bool force_shader_reload = game.ticks % 60 == 0 && game.debug_auto_shader_reload;
    if (IsKeyPressed(KEY_F5) || force_shader_reload)
    {
      game.dither_fx.reload();
      game.generate_palette_texture();
    }

    auto players     = get_components<Player>();
    bool move_camera = true;
#if defined(DEBUG)
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
      move_camera = false;
#endif
    if (!players.empty() && move_camera)
    {
      const auto &player   = players.front();
      auto &player_physics = get_component<Physics>(player.entity).get();
      auto &player_x       = player_physics.x;
      auto &player_y       = player_physics.y;
      camera.offset        = { game.render_texture.value.texture.width / 2.0f,
                               game.render_texture.value.texture.height / 2.0f };
      camera.target.x      = player_x;
      camera.target.y      = player_y - player_physics.mask.height / 2;
      camera.target.x      = roundf(camera.target.x);
      camera.target.y      = roundf(camera.target.y);

      if (camera.target.x < camera.offset.x)
        camera.target.x = camera.offset.x;
      if (camera.target.y < camera.offset.y)
        camera.target.y = camera.offset.y;
      if (camera.target.x > game.level.get_width() - camera.offset.x)
        camera.target.x = game.level.get_width() - camera.offset.x;
      if (camera.target.y > game.level.get_height() - camera.offset.y)
        camera.target.y = game.level.get_height() - camera.offset.y;

      camera.zoom     = 1.0f;
      camera.rotation = 0.0f;
    }

    static auto tileset        = LoadTexture("assets/tileset.png");
    static auto tileset_normal = LoadTexture("assets/tileset_normal.png");

    // draw
    if (game.dither_fx.enable())
    {
      BeginMode2D(camera);
      {
        auto &shader        = game.dither_fx.shader.shader;
        const auto res_vec2 = Vector2{ static_cast<float>(game_render_texture.texture.width),
                                       static_cast<float>(game_render_texture.texture.height) };
        SetShaderValue(shader, GetShaderLocation(shader, "resolution"), &res_vec2, SHADER_UNIFORM_VEC2);

        const auto lightPos_loc = GetShaderLocation(shader, "lightPos");
        assert(lightPos_loc != -1);
        SetShaderValueV(shader, lightPos_loc, &light_pos, SHADER_UNIFORM_VEC2, MAX_LIGHTS);

        const auto cameraPos_loc = GetShaderLocation(shader, "cameraPos");
        assert(cameraPos_loc != -1);
        Vector2 camera_pos = { camera.target.x - camera.offset.x, camera.target.y - camera.offset.y };
        SetShaderValue(shader, cameraPos_loc, &camera_pos, SHADER_UNIFORM_VEC2);

        const auto lightSize_loc = GetShaderLocation(shader, "lightSize");
        assert(lightSize_loc != -1);
        SetShaderValueV(shader, lightSize_loc, &light_size, SHADER_UNIFORM_FLOAT, MAX_LIGHTS);

        const auto lightIntensity_loc = GetShaderLocation(shader, "lightIntensity");
        assert(lightIntensity_loc != -1);
        SetShaderValueV(shader, lightIntensity_loc, &light_intensity, SHADER_UNIFORM_FLOAT, MAX_LIGHTS);

        SetShaderValueTexture(shader, GetShaderLocation(shader, "texture0"), tileset);
        SetShaderValueTexture(shader, GetShaderLocation(shader, "texture1"), tileset_normal);
        SetShaderValueTexture(shader, GetShaderLocation(shader, "texture2"), game.palette_texture);
        ClearBackground(PALETTE_WHITE);

        manager.call_render(0);
        manager.call_render(NEG_INF_DEPTH);
      }
      EndMode2D();

      game.dither_fx.disable();
    }

    BeginTextureMode(game_render_texture);
    {
      game.dither_fx.draw_texture(target_source(game_render_texture));

#if defined(DEBUG)
      DrawTexture(game.palette_texture, 2, 2, WHITE);
      DrawText(TextFormat("Lights: %zu", light_idx), 2, 12 + 1, 10, PALETTE_BLACK);
      DrawText(TextFormat("Lights: %zu", light_idx), 2 + 1, 12, 10, PALETTE_GRAY);
      DrawText(TextFormat("Lights: %zu", light_idx), 2, 12, 10, PALETTE_YELLOW);
#endif
    }
    EndTextureMode();

    game.values.frame_progress = frame_progress;

    [[maybe_unused]] const auto mouse_position       = INPUT.interface_mouse();
    [[maybe_unused]] const auto &interface_texture_w = interface_render_texture.texture.width;
    [[maybe_unused]] const auto &interface_texture_h = interface_render_texture.texture.height;
    BeginTextureMode(interface_render_texture);
    {
      ClearBackground(BLANK);
#if defined(DEBUG)
      const Color cursor_color = PALETTE_BLUE;
      INPUT.hide_cursor();

      [[maybe_unused]] auto mouse_position = round(INPUT.interface_mouse());

      static Vector2 previous_mouse_position = mouse_position;
      static size_t mouse_moved_tick         = Game::get_ticks();
      if (!Vector2Equals(previous_mouse_position, mouse_position))
        mouse_moved_tick = Game::get_ticks();

      const bool mouse_moved = Game::get_ticks() - mouse_moved_tick < 60 * 2;
      static float scale     = 1.0f;
      // draw mouse
      if (mouse_moved)
        scale = Lerp(scale, 2.0f, 0.08f);
      else
        scale = Lerp(scale, 0.1f, 0.12f);

      if (scale > 0.2f)
      {
        const float mx = std::roundf(mouse_position.x);
        const float my = std::roundf(mouse_position.y);
        const float ts = std::roundf(8.0f) * scale;
        const float ls = ts * 0.61f;
        DrawTriangle({ mx, my }, { mx, my + ts }, { mx + ts, my }, cursor_color);
        DrawCircleV({ mx + ls, my + ls }, 2.0f * scale, cursor_color);
      }

      previous_mouse_position = mouse_position;
#endif
    }
    EndTextureMode();
  }

  void G_update_game()
  {
    assert(game && "Game is not created");

    game->ticks += 1;

    auto &manager = Manager::get();

    const auto &players = get_components<Player>();
    if (!players.empty() && game)
    {
      const auto &player   = players.front();
      auto &player_physics = get_component<Physics>(player.entity).get();
      auto &player_x       = player_physics.x;
      auto &player_y       = player_physics.y;

      const auto level_width  = game->level.get_width();
      const auto level_height = game->level.get_height();
      bool level_changed      = false;

      if (player_x < 4)
      {
        game->level.load_neighbour(Level::Direction::West);
        player_x      = game->level.get_width() - 5;
        level_changed = true;
      }
      else if (player_x >= level_width - 4)
      {
        game->level.load_neighbour(Level::Direction::East);
        player_x      = 5;
        level_changed = true;
      }
      else if (player_y < 0)
      {
        game->level.load_neighbour(Level::Direction::North);
        player_y      = game->level.get_height() - 16;
        level_changed = true;
      }
      else if (player_y >= level_height)
      {
        game->level.load_neighbour(Level::Direction::South);
        player_y      = 2;
        level_changed = true;
      }

      if (level_changed)
      {
        game->particle_system.get().clear();
        manager.call_init();
      }

#if defined(DEBUG)
      auto mouse_position = INPUT.game_mouse();
      if (INPUT.mouse_right)
      {
        player_x = mouse_position.x - game->camera.offset.x + game->camera.target.x;
        player_y = mouse_position.y - game->camera.offset.y + game->camera.target.y;
      }

      if (IsKeyDown(KEY_LEFT_ALT))
      {
        if (INPUT.left)
          player_x -= level_width / 12;
        if (INPUT.right)
          player_x += level_width / 12;
        if (INPUT.up)
          player_y -= level_height / 12;
        if (INPUT.down)
          player_y += level_height / 12;
      }
#endif
    }

    if (INPUT.mute.pressed())
      game->mute = !game->mute;

    if (game->mute && IsMusicStreamPlaying(game->music))
      StopMusicStream(game->music);
    if (!game->mute && !IsMusicStreamPlaying(game->music))
      PlayMusicStream(game->music);

    manager.call_destroy();
    manager.call_init();

    if (game->skip_ticks_count > 0)
    {
      game->skip_ticks_count = std::max(game->skip_ticks_count - 1ul, 0ul);
      return;
    }

    for (auto &component_id : manager.preupdate_components)
    {
      auto &container = manager.component_containers[component_id];
      if (container.preupdate)
        container.preupdate();
    }

    for (auto &component_id : manager.update_components)
    {
      auto &container = manager.component_containers[component_id];
      if (container.update)
        container.update();
    }

    for (auto &component_id : manager.postupdate_components)
    {
      auto &container = manager.component_containers[component_id];
      if (container.postupdate)
        container.postupdate();
    }

    manager.call_init();

    game->update_timers();
  }
};

Game &Game::get()
{
  assert(game && "Game is not created");
  return *game;
}

Game::Game()
{
  music = LoadMusicStream("assets/music/electric-chill-pop.mp3");
  PlayMusicStream(music);

  generate_palette_texture();
}

double Game::frame_progress()
{
  return std::clamp(get().values.frame_progress, 0.1, 1.0);
}

void Game::add_timer(Entity entity, std::function<void()> &&callback, size_t frames)
{
  get().timers.push_back({ entity, std::move(callback), frames });
}

void Game::add_particles(int x, int y, const Particle &type, size_t count)
{
  auto &sys = get().particle_system.get();
  for (size_t i = 0; i < count; i++)
  {
    sys.add_particle(x, y, type);
  }
}

ParticleBuilder Game::particle_builder()
{
  assert(get().particle_system && "Particle system is not created");
  return ParticleBuilder(get().particle_system.get());
}

void Game::update_timers()
{
  std::erase_if(timers, [](const Timer &timer) { return !entity_exists(timer.entity); });

  for (auto &[entity, callback, frames] : timers)
  {
    frames -= 1;
    if (frames == 0)
      callback();
  }

  std::erase_if(timers, [](const Timer &timer) { return timer.frames <= 0; });
}

size_t Game::get_ticks()
{
  return get().ticks;
}

void Game::generate_palette_texture()
{
  Image palette_image = GenImageColor(8, 8, BLANK);

  [[maybe_unused]] const Color default_colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY,   PALETTE_BLUE,
                                                  PALETTE_GREEN, PALETTE_RED,    PALETTE_YELLOW, PALETTE_WHITE };
  // black
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_BLACK, PALETTE_PURPLE,
                     PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY,  PALETTE_BLUE };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 0, i, colors[i]);
  }

  // purple
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY,  PALETTE_PURPLE,
                     PALETTE_BLUE,  PALETTE_PURPLE, PALETTE_GREEN, PALETTE_RED };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 1, i, colors[i]);
  }

  // gray
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY,   PALETTE_BLUE,
                     PALETTE_GREEN, PALETTE_RED,    PALETTE_YELLOW, PALETTE_WHITE };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 2, i, colors[i]);
  }

  // blue
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY, PALETTE_BLUE,
                     PALETTE_GREEN, PALETTE_BLUE,   PALETTE_BLUE, PALETTE_WHITE };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 3, i, colors[i]);
  }

  // green
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY,  PALETTE_GREEN,
                     PALETTE_GREEN, PALETTE_GREEN,  PALETTE_GREEN, PALETTE_WHITE };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 4, i, colors[i]);
  }

  // red
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY, PALETTE_BLUE,
                     PALETTE_RED,   PALETTE_YELLOW, PALETTE_RED,  PALETTE_WHITE };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 5, i, colors[i]);
  }

  // yellow
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY,   PALETTE_BLUE,
                     PALETTE_RED,   PALETTE_YELLOW, PALETTE_YELLOW, PALETTE_WHITE };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 6, i, colors[i]);
  }

  // white
  {
    Color colors[8]{ PALETTE_BLACK, PALETTE_PURPLE, PALETTE_GRAY,   PALETTE_BLUE,
                     PALETTE_RED,   PALETTE_WHITE,  PALETTE_YELLOW, PALETTE_WHITE };
    for (size_t i = 0; i < 8; i++)
      ImageDrawPixel(&palette_image, 7, i, colors[i]);
  }

  palette_texture = LoadTextureFromImage(palette_image);

  UnloadImage(palette_image);
}

int64_t Game::level_width()
{
  return get().level.get_width();
}

int64_t Game::level_height()
{
  return get().level.get_height();
}

std::string_view Game::level_name()
{
  return get().level.get_name();
}

void Game::skip_ticks(size_t count)
{
  get().skip_ticks_count += count;
}
