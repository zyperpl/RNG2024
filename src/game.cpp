#include "game.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>

#include <raylib.h>

#include "component.hpp"
#include "hurtable.hpp"
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
    game->init();
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
    {
      auto &sprite =
        game->particle_system.get().get_sprite(game->particle_system.get().sprite_id("assets/tileset.png:bit"));
      sprite.source_offset.x = 8 + 8 + 8;
      sprite.source_offset.y = 216;
      sprite.set_frame_width(8);
      sprite.set_frame_height(8);
      sprite.set_frame_count(1);
    }

    auto &manager = Manager::get();
    manager.call_init();
    game->level.load("Area_Zero");
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
    auto clear_lights = [&]()
    {
      for (size_t i = 0; i < MAX_LIGHTS; i++)
      {
        light_pos[i]       = Vector2{ -990.0f, -990.0f };
        light_size[i]      = 0.0f;
        light_intensity[i] = 0.0f;
        light_size[i]      = 1.0f;
      }
    };
    clear_lights();
    size_t light_idx                  = 0;
    auto &camera                      = game.camera;
    light_pos[light_idx].x            = Input::get().game_mouse().x - camera.offset.x + camera.target.x;
    light_pos[light_idx].y            = Input::get().game_mouse().y - camera.offset.y + camera.target.y;

#if defined(DEBUG)
    static float mouse_light_strength = 0.0f;
    if (Input::get().mouse_wheel_down)
      mouse_light_strength -= 0.2f;
    if (Input::get().mouse_wheel_up)
      mouse_light_strength += 0.2f;
    if (mouse_light_strength < 0.0f)
      mouse_light_strength = 0.0f;
    light_intensity[light_idx] = mouse_light_strength;
    light_idx += 1;
#endif

    for (const auto &light : get_components<Light>())
    {
      if (light.x + 20 + light.intensity * 16 + light.size * 12 < camera.target.x - camera.offset.x ||
          light.x - 20 - light.intensity * 16 - light.size * 12 > camera.target.x + camera.offset.x ||
          light.y + 20 + light.intensity * 8 + light.size * 8 < camera.target.y - camera.offset.y ||
          light.y - 20 - light.intensity * 8 - light.size * 8 > camera.target.y + camera.offset.y)
        continue;

      if (light.size < 0.001f)
        continue;

      if (light.intensity < 0.01f)
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
      camera.target.x      = player_x - player_physics.mask.width / 2;
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

    // map variables
    static auto tileset        = LoadTexture("assets/tileset.png");
    static auto tileset_normal = LoadTexture("assets/tileset_normal.png");

    // draw
    if (game.dither_fx.enable())
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

      BeginMode2D(camera);
      {
        manager.call_render();
      }
      EndMode2D();

      game.dither_fx.disable();
    }

    BeginTextureMode(game_render_texture);
    {
      ClearBackground(PALETTE_BLUE);

      const auto w           = game_render_texture.texture.width;
      static float cloud_x   = 0.0f;
      float cloud_y          = 120.0f;
      cloud_x                = fmod(Game::tick() * 0.5f - camera.target.x, w);
      const auto cloud_color = PALETTE_WHITE;
      for (int i = -2; i < 4; i++)
      {
        DrawCircle(cloud_x + i * w, cloud_y, 60, cloud_color);
        DrawCircle(cloud_x + 80 + i * w, cloud_y - 20, 60, cloud_color);
        DrawCircle(cloud_x + 80 + i * w, cloud_y + 60, 120, cloud_color);
        DrawCircle(cloud_x + 240 + i * w, cloud_y + 60, 100, cloud_color);
        DrawCircle(cloud_x + 160 + i * w, cloud_y + 20, 60, cloud_color);
      }

      Rectangle game_screen_dest = target_source(game_render_texture);
      game_screen_dest.y -= (game.map_x - 400) * 2.5;
      game.dither_fx.draw_texture(game_screen_dest);

      BeginMode2D(camera);
      {
        game.draw_deferred();
        game.draw();
      }
      EndMode2D();

#if defined(DEBUG)
      DrawTexture(game.palette_texture, 2, 2, FULLWHITE);
      DrawTextEx(
        game.font, TextFormat("Lights: %zu", light_idx), { 2, 12 }, game.font_size, game.font_spacing, PALETTE_YELLOW);
#endif
    }
    EndTextureMode();

    game.values.frame_progress = frame_progress;

    [[maybe_unused]] const auto mouse_position       = INPUT.interface_mouse();
    [[maybe_unused]] const auto &interface_texture_w = interface_render_texture.texture.width;
    [[maybe_unused]] const auto &interface_texture_h = interface_render_texture.texture.height;

    clear_lights();
    light_idx                  = 0;
    light_pos[light_idx].x     = 5.0f;
    light_pos[light_idx].y     = interface_texture_h / 2.0f;
    light_size[light_idx]      = 3.7f;
    light_intensity[light_idx] = 1.0f;

    light_idx++;
    light_pos[light_idx].x     = 0.0f;
    light_pos[light_idx].y     = interface_texture_h / 2.0f + 40.0f;
    light_size[light_idx]      = 1.0f;
    light_intensity[light_idx] = 0.6f;

    light_idx++;
    light_pos[light_idx].x     = -20.0f;
    light_pos[light_idx].y     = interface_texture_h - 40.0f;
    light_size[light_idx]      = 1.0f;
    light_intensity[light_idx] = 1.0f;

    light_idx++;
    light_pos[light_idx].x     = interface_texture_w / 2.0f;
    light_pos[light_idx].y     = interface_texture_h / 2.0f;
    light_size[light_idx]      = 600.0f;
    light_intensity[light_idx] = 0.6f;

    if (game.dither_fx.enable())
    {
      auto &shader        = game.dither_fx.shader.shader;
      const auto res_vec2 = Vector2{ static_cast<float>(interface_render_texture.texture.width),
                                     static_cast<float>(interface_render_texture.texture.height) };
      SetShaderValue(shader, GetShaderLocation(shader, "resolution"), &res_vec2, SHADER_UNIFORM_VEC2);

      const auto lightPos_loc = GetShaderLocation(shader, "lightPos");
      assert(lightPos_loc != -1);
      SetShaderValueV(shader, lightPos_loc, &light_pos, SHADER_UNIFORM_VEC2, MAX_LIGHTS);

      const auto cameraPos_loc = GetShaderLocation(shader, "cameraPos");
      assert(cameraPos_loc != -1);
      Vector2 camera_pos{ 0, 0 };
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

      ClearBackground(BLANK);
      static Sprite energy_bar_bg("assets/tileset.png");
      static Sprite energy_bar("assets/tileset.png");
      energy_bar_bg.set_frame_width(8);
      energy_bar_bg.set_frame_height(4);
      energy_bar_bg.source_offset.x = 0;
      energy_bar_bg.source_offset.y = 172;
      energy_bar_bg.set_frame_count(1);

      energy_bar.set_frame_width(8);
      energy_bar.set_frame_height(energy_bar_bg.get_height());
      energy_bar.source_offset.x = energy_bar_bg.get_width();

      energy_bar.source_offset.y = energy_bar_bg.source_offset.y;
      energy_bar.set_frame_count(6);
      energy_bar.set_frame_durations(120);
      energy_bar.animate();

      if (!players.empty())
      {
        const auto &player         = players.front();
        const auto player_hurtable = get_component<Hurtable>(player.entity).get();

        const int health     = player_hurtable.health;
        const int max_health = player_hurtable.max_health;

        static int draw_health = health;

        if (draw_health < health || health < max_health * 0.3)
          draw_health = health;
        else
          draw_health = Lerp(draw_health, health, 0.1f);

        const int cell_h     = energy_bar.get_height();
        const int bar_height = max_health * cell_h;
        static int xoffset   = 0;
        int yoffset          = (health <= max_health * 0.35) ? (Game::tick() / 4 % 2) : 0;
        if (auto player_hurtable_ref = get_component<Hurtable>(player.entity))
        {
          if (player_hurtable_ref.get().hurt_frames_ago() < 16)
            xoffset = 2;
        }
        xoffset = Lerp(xoffset, 0, 0.1f);

        for (int i = 0; i < max_health; i++)
        {
          const float ex = 2.0f + xoffset;
          const float ey = interface_texture_h / 2.0f + yoffset + bar_height / 2.0f - i * cell_h;

          if (i < draw_health)
          {
            energy_bar.position.x = ex;
            energy_bar.position.y = ey;
            energy_bar.draw();
          }

          energy_bar_bg.position.x = ex;
          energy_bar_bg.position.y = ey;
          energy_bar_bg.draw();
        }
      }

      static const NPatchInfo npatchinfo{ Rectangle{ 0.0f, 104.0f, 24.0f, 24.0f }, 8, 8, 8, 8, NPATCH_NINE_PATCH };
      if (game.has_messages() || game.show_map)
      {
        const auto canvas_w = game.render_texture.value.texture.width;
        const auto canvas_h = game.render_texture.value.texture.height;

        const int border = 20;
        int dialog_w     = canvas_w - border * 4;
        int dialog_h     = 40;
        int dialog_x     = canvas_w / 2 - dialog_w / 2;
        int dialog_y     = canvas_h - 30 - border;

        const Rectangle dialog_rect{ static_cast<float>(dialog_x),
                                     static_cast<float>(dialog_y),
                                     static_cast<float>(dialog_w),
                                     static_cast<float>(dialog_h) };

        DrawTextureNPatch(tileset, npatchinfo, dialog_rect, Vector2{ 0, 0 }, 0.0f, FULLWHITE);
      }

      DrawTextureNPatch(tileset,
                        npatchinfo,
                        Rectangle{ static_cast<float>(game.map_x + 16),
                                   static_cast<float>(game.map_y - 4),
                                   static_cast<float>(game.map_texture.width - 40),
                                   static_cast<float>(game.map_texture.height) - 56 },
                        Vector2{ 0, 0 },
                        0.0f,
                        FULLWHITE);
      DrawTextureNPatch(tileset,
                        npatchinfo,
                        Rectangle{ static_cast<float>(game.map_x - 62),
                                   static_cast<float>(game.map_y - 4),
                                   static_cast<float>(76),
                                   static_cast<float>(game.map_texture.height) - 56 },
                        Vector2{ 0, 0 },
                        0.0f,
                        FULLWHITE);

      game.dither_fx.disable();
    }

    BeginTextureMode(interface_render_texture);
    {
      ClearBackground(BLANK);

      game.dither_fx.draw_texture(target_source(interface_render_texture));

      game.draw_map();

      if (game.has_messages())
        game.draw_messages();

      if (game.game_over)
      {
        Rectangle rect{ 0, 0, 160, 60 };
        rect.x = 320 / 2 - rect.width / 2;
        rect.y = 180 / 2 - rect.height / 2 + 20;
        DrawRectangleRounded(rect, 0.1f, 8, PALETTE_BLACK);

        const char *text     = "Game over. You win!";
        const auto text_size = MeasureTextEx(game.font, text, game.font_size, game.font_spacing).x;
        DrawTextEx(game.font,
                   text,
                   { roundf(static_cast<float>(320 / 2 - text_size / 2)), 180 / 2 },
                   game.font_size,
                   game.font_spacing,
                   PALETTE_BLACK);
        DrawTextEx(game.font,
                   text,
                   { roundf(static_cast<float>(320 / 2 - text_size / 2)), 180 / 2 + 1.0f },
                   game.font_size,
                   game.font_spacing,
                   PALETTE_WHITE);
        DrawTextEx(game.font,
                   text,
                   { roundf(static_cast<float>(320 / 2 - text_size / 2)), 180 / 2 + 2.0f },
                   game.font_size,
                   game.font_spacing,
                   PALETTE[game.tick() % 8]);

        const auto &players = get_components<Player>();
        if (!players.empty())
        {
          const auto &player = players.front();
          auto s = std::chrono::duration_cast<std::chrono::seconds>(game.end_time - player.init_time).count();

          DrawTextEx(game.font,
                     TextFormat("It took you %d s", s),
                     { 320 / 2 - 54, 180 / 2 + 10 - 1 },
                     game.font_size,
                     game.font_spacing,
                     PALETTE_BLACK);
          DrawTextEx(game.font,
                     TextFormat("It took you %d s", s),
                     { 320 / 2 - 54, 180 / 2 + 10 },
                     game.font_size,
                     game.font_spacing,
                     PALETTE_WHITE);

          if (s < 300)
          {
            DrawTextEx(game.font,
                       "You are a pro",
                       { roundf(320 / 2 - 40 + randi(0, 3)), 180 / 2 + 30 - 1 },
                       game.font_size,
                       game.font_spacing,
                       PALETTE_BLACK);
            DrawTextEx(game.font,
                       "You are a pro",
                       { roundf(320 / 2 - 40 + randi(0, 3)), 180 / 2 + 30 },
                       game.font_size,
                       game.font_spacing,
                       PALETTE[game.tick() % 8]);
          }
        }
      }

#if defined(DEBUG)
      const Color cursor_color = PALETTE_BLUE;
      INPUT.hide_cursor();

      [[maybe_unused]] auto mouse_position = round(INPUT.interface_mouse());

      static Vector2 previous_mouse_position = mouse_position;
      static size_t mouse_moved_tick         = Game::tick();
      if (!Vector2Equals(previous_mouse_position, mouse_position))
        mouse_moved_tick = Game::tick();

      const bool mouse_moved = Game::tick() - mouse_moved_tick < 60 * 2;
      static float scale     = 1.0f;
      // draw mouse
      if (mouse_moved)
        scale = Lerp(scale, 0.6f, 0.08f);
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

    game->action_pressed = INPUT.jump.pressed() || INPUT.shoot.pressed() || INPUT.special.pressed();

    const auto &players = get_components<Player>();
    if (!players.empty() && game)
    {
      auto &player          = players.front();
      auto &player_hurtable = get_component<Hurtable>(player.entity).get();
      auto &player_physics  = get_component<Physics>(player.entity).get();
      auto &player_x        = player_physics.x;
      auto &player_y        = player_physics.y;

      const auto level_width      = game->level.get_width();
      const auto level_height     = game->level.get_height();
      const auto cur_level_x      = game->level.get_world_x();
      const auto cur_level_y      = game->level.get_world_y();
      const auto cur_level_width  = game->level.get_width();
      const auto cur_level_height = game->level.get_height();
      std::string cur_level_name  = game->level.get_name();

      bool level_changed = false;

      const int x_offset = 4;
      const int y_offset = 4;

      if (player_hurtable.health <= 0)
      {
        if (game->defeated_frames <= 0)
        {
          game->defeated_frames = game->defeated_max_frames;
          player.boom_sound.play();
        }
      }
      else if (player_x < x_offset)
      {
        game->level.load_neighbour(Level::Direction::West);
        player_x = game->level.get_width() - x_offset;
        player_y += cur_level_y - game->level.get_world_y();
        level_changed = cur_level_name != game->level.get_name();
      }
      else if (player_x >= cur_level_width + x_offset)
      {
        game->level.load_neighbour(Level::Direction::East);
        player_x = x_offset;
        player_y += cur_level_y - game->level.get_world_y();
        level_changed = cur_level_name != game->level.get_name();
      }
      else if (player_y < y_offset - 4)
      {
        printf("Loading north\n");

        game->level.load_neighbour(Level::Direction::North);
        player_y = game->level.get_height();
        player_x += cur_level_x - game->level.get_world_x();
        level_changed = cur_level_name != game->level.get_name();
      }
      else if (player_y >= cur_level_height + y_offset)
      {
        printf("Loading south\n");

        game->level.load_neighbour(Level::Direction::South);
        player_y = y_offset + 4;
        player_x += cur_level_x - game->level.get_world_x();
        level_changed = cur_level_name != game->level.get_name();

        printf("Previous level name: %s\n", cur_level_name.data());
        printf("Current level name: %s\n", game->level.get_name().data());

        if (!level_changed)
        {
          player_hurtable.hurt(1000, player_x, player_y);
          if (game->defeated_frames <= 0)
          {
            game->defeated_frames = game->defeated_max_frames;
            player.boom_sound.play();
          }
        }
      }
      else if (player_physics.is_colliding_with_solid())
      {
#if !defined(DEBUG)
        printf("Incorrect player position: %d, %d\n", player_x, player_y);
        game->queue_message("Something strange happened", PALETTE_BLACK);
        player_hurtable.hurt(1000, player_x, player_y);
#endif
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
          player_x -= level_width / 32;
        if (INPUT.right)
          player_x += level_width / 32;
        if (INPUT.up)
          player_y -= level_height / 24;
        if (INPUT.down)
          player_y += level_height / 24;
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

    if (!game->has_messages() && !game->show_map)
    {
      if (game->defeated_frames > 0)
      {
        static Particle particle = game->particle_builder()
                                     .sprite("assets/tileset.png:star")
                                     .size(1.0f)
                                     .life(10, 20)
                                     .velocity({ -1.0f, -4.0f }, { 1.0f, 1.0f })
                                     .build();

        auto &player_physics = get_component<Physics>(players.front().entity).get();
        game->add_particles(player_physics.x_previous, player_physics.y_previous - randi(0, 50), particle, 4);

        // hack
        for (auto &particle_system : get_components<ParticleSystem>())
        {
          particle_system.update();
        }

        game->defeated_frames -= 1;
        printf("Defeated frames: %d\n", game->defeated_frames);
        if (game->defeated_frames == 1)
        {
          game->defeated_frames = 0;
          game->queue_message("You were destroyed");
          game->load_selected_level();

          auto &player_hurtable  = get_component<Hurtable>(players.front().entity).get();
          player_hurtable.health = player_hurtable.max_health;
        }

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
    }
    else
    {
      game->update_messages();
    }

#if defined(DEBUG)
    if (IsKeyPressed(KEY_B))
    {
      game->show_map = !game->show_map;
    }
    if (IsKeyPressed(KEY_N))
    {
      auto next_idx = game->selected_map_node + 1;
      if (next_idx >= game->map_nodes.size())
        next_idx = 0;

      game->map_nodes[game->selected_map_node].discovered = true;
      game->map_nodes[game->selected_map_node].completed  = true;
      game->map_nodes[next_idx].discovered                = true;
    }
#endif

    game->update_map();

    manager.call_init();

    game->update_timers();
  }
};

Game &Game::get()
{
  assert(game && "Game is not created");
  return *game;
}

Game::Game() {}

void Game::init()
{
  music = LoadMusicStream("assets/music/electric-chill-pop.mp3");
  PlayMusicStream(music);

  generate_palette_texture();

  if (!IsFontValid(font))
    UnloadFont(font);

  font = LoadFontEx("assets/KubastaFixed.ttf", font_size, nullptr, 0);

  // NOTE: Some pixelart fonts are anti-aliased.
  // On the Web we cannot retrieve image from the GPU texture, possibly due to a bug or WebGL limitation.
#if !defined(EMSCRIPTEN)
  auto font_image = LoadImageFromTexture(font.texture);
  ImageFormat(&font_image, PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA);
  for (int y = 0; y < font_image.height; y++)
  {
    for (int x = 0; x < font_image.width; x++)
    {
      Color pixel = GetImageColor(font_image, x, y);
      if (pixel.a != 0 && pixel.a != 255)
      {
        pixel.a = pixel.a > 192 ? 255 : 0;
        pixel.r = 255;
        pixel.g = 255;
        pixel.b = 255;
        ImageDrawPixel(&font_image, x, y, pixel);
      }
    }
  }
  // ExportImage(font_image, "assets/KubastaFixed.png");
  font.texture = LoadTextureFromImage(font_image);
  UnloadImage(font_image);

#else
  UnloadTexture(font.texture);
  font.texture = LoadTexture("assets/KubastaFixed.png");
#endif

  assert(IsFontValid(font) && "Font is not valid");

  char_sound   = GameSound("assets/sounds/blip.wav");
  select_sound = GameSound("assets/sounds/blip2.wav");
  up_sound     = GameSound("assets/sounds/up2.wav");
  up2_sound     = GameSound("assets/sounds/up3.wav");
}

void Game::draw()
{
  auto players = get_components<Player>();

  if (!players.empty())
  {
    const auto &player = players.front();
    if (player.can_interact)
    {
      auto text      = "[C] INTERACT";
      auto text_size = MeasureTextEx(font, text, font_size, font_spacing);
      DrawTextEx(font,
                 text,
                 { player.interact_point.x - text_size.x / 2, player.interact_point.y - 20 + 1 },
                 font_size,
                 font_spacing,
                 PALETTE_BLACK);
      DrawTextEx(font,
                 text,
                 { player.interact_point.x - text_size.x / 2 - 1, player.interact_point.y - 20 },
                 font_size,
                 font_spacing,
                 PALETTE_BLACK);
      DrawTextEx(font,
                 text,
                 { player.interact_point.x - text_size.x / 2 + 1, player.interact_point.y - 20 },
                 font_size,
                 font_spacing,
                 PALETTE_BLACK);
      DrawTextEx(font,
                 text,
                 { player.interact_point.x - text_size.x / 2, player.interact_point.y - 20 - 1 },
                 font_size,
                 font_spacing,
                 PALETTE_BLACK);
      DrawTextEx(font,
                 text,
                 { player.interact_point.x - text_size.x / 2, player.interact_point.y - 20 },
                 font_size,
                 font_spacing,
                 PALETTE_WHITE);
    }
  }
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

size_t Game::tick()
{
  return get().ticks;
}

size_t Game::frame()
{
  return get().frames;
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

std::string Game::level_name()
{
  return get().level.get_name();
}

void Game::skip_ticks(size_t count)
{
  get().skip_ticks_count += count;
}

bool Game::on_screen(Rectangle rect)
{
  rect                     = inflate(rect, 8);
  const auto &g            = get();
  const auto camera_left   = g.camera.target.x - g.camera.offset.x;
  const auto camera_top    = g.camera.target.y - g.camera.offset.y;
  const auto camera_right  = camera_left + g.render_texture.value.texture.width;
  const auto camera_bottom = camera_top + g.render_texture.value.texture.height;

  return rect.x + rect.width >= camera_left && rect.x <= camera_right && rect.y + rect.height >= camera_top &&
         rect.y <= camera_bottom;
}

void Game::defer_draw(Entity entity, std::function<void()> &&callback)
{
  get().deferred_draws.push_back({ entity, std::move(callback) });
}

void Game::draw_deferred()
{
  for (auto &[entity, callback] : get().deferred_draws)
  {
    if (entity_exists(entity))
      callback();
  }
  get().deferred_draws.clear();
}

void Game::queue_message(std::string message, Color color)
{
  get().messages.push({ std::move(message), color });
}

void Game::end_level()
{
  auto &g = get();
  if (g.level_name() == "Level_1")
  {
    const Color color{ PALETTE_YELLOW };
    queue_message("#show_map");
    queue_message("#complete 0");
    queue_message("#discover 0");
    queue_message("Hello?|Can you hear me?", color);
    queue_message("I see you restored the connection in this area.", color);
    queue_message("I'm glad you're here. I need your help.", color);
    queue_message("Come find me, I'm north-east from your position.", color);
    queue_message("#discover 1");
    queue_message("Oh, and one more thing, be sure to-", color);

    queue_message("Transmission lost", PALETTE_WHITE);
    queue_message("New connected area available", PALETTE_WHITE);
  }
  else if (g.level_name() == "Level_8")
  {
    const Color color{ PALETTE_YELLOW };
    queue_message("#show_map");
    queue_message("#complete 1");
    queue_message("#discover 1");
    queue_message("By reconnecting this area, you've secured critical support for the people below.", color);
    queue_message("However, to ensure our long-term survival, we must reclaim greenhouses in the north.", color);
    queue_message("#discover 2");
    queue_message("Our lives depend on it. Good luck!", color);

    queue_message("Transmission ended", PALETTE_WHITE);
    queue_message("New area connected", PALETTE_WHITE);
  }
  else if (g.level_name() == "Level_16")
  {
    queue_message("#show_map");
    queue_message("#complete 2");
    queue_message("#discover 2");
    queue_message("Detected anomalous power source.", PALETTE_WHITE);
    queue_message("#discover 3");
    queue_message("New connected area available", PALETTE_WHITE);
  }
  else if (g.level_name() == "Level_19")
  {
    const Color color{ PALETTE_YELLOW };
    queue_message("#show_map");
    queue_message("#complete 3");
    queue_message("#discover 3");
    queue_message("All areas reconnected.");
    queue_message("You've done it! wow wow nice", color);
    queue_message("#complete 4");
  }
}

void Game::update_messages()
{
  if (messages.empty())
    return;

  if (messages.front().first.starts_with('#'))
  {
    const auto &message = messages.front().first;

    if (message.starts_with("#complete"))
    {
      const auto parts = split(message, ' ');
      if (parts.size() == 2)
      {
        const size_t node_id = std::stoi(parts[1]);

        // hack, special case
        if (node_id == 4)
        {
          game_over = true;
          end_time  = std::chrono::high_resolution_clock::now();
          messages.pop();
          return;
        }

        if (node_id < map_nodes.size())
        {
          map_nodes[node_id].completed = true;
        }
      }
      messages.pop();
      return;
    }

    if (message == "#show_map")
    {
      show_map = true;
      messages.pop();
      return;
    }

    if (message.starts_with("#discover"))
    {
      const auto parts = split(message, ' ');
      if (parts.size() == 2)
      {
        const size_t node_id = std::stoi(parts[1]);
        if (node_id < map_nodes.size())
        {
          map_nodes[node_id].discovered = true;
          up_sound.play();

          selected_map_node = node_id;
        }
      }
      messages.pop();
      return;
    }
  }

  static int max_delay         = 4;
  static bool skipping_message = false;
  if (!message_ready && action_pressed)
  {
    skipping_message = true;
  }

  if (!message_ready)
  {
    if (drawn_message.first != messages.front().first)
    {
      static int delay = 0;
      if (delay == 0)
      {
        delay                = max_delay;
        drawn_message.first  = messages.front().first.substr(0, drawn_message.first.size() + 1);
        drawn_message.second = messages.front().second;

        char_sound.set_pitch(1.0f + randf(-0.2f, 0.1f));
        if (drawn_message.second != PALETTE_WHITE)
          char_sound.set_pitch(1.0f + randf(0.1f, 0.6f));
        char_sound.play();
      }
      else
        delay -= 1;

      if (skipping_message)
        delay = 0;
    }
    else
    {
      message_ready = true;
      drawn_message = messages.front();
    }
  }
  else
  {
    if (action_pressed)
    {
      action_pressed   = false;
      skipping_message = false;
      messages.pop();
      message_ready = false;
      drawn_message.first.clear();
      drawn_message.second = PALETTE_WHITE;

      select_sound.play();

      while (!messages.empty() && messages.front().first.empty())
        messages.pop();

      // prepare next message immediately
      if (!messages.empty())
      {
        drawn_message.first  = messages.front().first.at(0);
        drawn_message.second = messages.front().second;
      }
    }
  }
}

void Game::draw_messages()
{
  if (drawn_message.first.empty())
    return;

  const auto canvas_w = render_texture.value.texture.width;
  const auto canvas_h = render_texture.value.texture.height;

  const int border   = 20;
  const int dialog_w = canvas_w - border * 4;
  const int dialog_h = 40;
  const int dialog_x = canvas_w / 2 - dialog_w / 2;
  const int dialog_y = canvas_h - 30 - border;

  const auto &message = drawn_message.first;
  const auto &color   = drawn_message.second;

  int tx = 0;
  int ty = 0;

  for (const auto &c : message)
  {
    auto txt         = TextFormat("%c", c);
    auto letter_size = MeasureTextEx(font, txt, font_size, font_spacing);

    if (*txt == '\0' || letter_size.x <= 0)
      continue;

    // hack
    bool new_line = c == '|';

    // hack
    if (c == ' ' && tx + 40 > dialog_x + dialog_w - 60)
      new_line = true;

    if (new_line)
    {
      tx = 0;
      ty += font_size / 2 + 2;
      continue;
    }

    DrawTextEx(font,
               txt,
               { dialog_x + 5.0f + tx, dialog_y + 1.0f + ty },
               font_size,
               font_spacing,
               color == PALETTE_WHITE ? PALETTE_PURPLE : PALETTE_BLACK);
    DrawTextEx(font, txt, { dialog_x + 5.0f + tx, dialog_y + 0.0f + ty }, font_size, font_spacing, color);

    tx += letter_size.x + font_spacing;
  }

  if (message_ready)
  {
    const std::string btn_txt = messages.size() > 1 ? ">" : "x";
    if (ticks / 10 % 2 == 0)
      DrawTextEx(font,
                 btn_txt.c_str(),
                 { dialog_x + dialog_w - 5.0f - 4.0f * btn_txt.size(), dialog_y + dialog_h - 16.0f },
                 font_size,
                 font_spacing,
                 PALETTE_YELLOW);
  }
}

void Game::update_map()
{
  if (!show_map)
  {
    if (map_x < 400)
      map_x += 4;
    else
      map_x = 400;

    return;
  }

  if (map_x > 104)
    map_x -= 4;
  else
    map_x = 104;

  for (auto &n : map_nodes)
  {
    if (!n.discovered)
      continue;

    if (n.draw_radius > 4)
      n.draw_radius -= 1.0f;
  }

  if (INPUT.left.pressed())
  {
    if (selected_map_node > 0)
    {
      selected_map_node -= 1;
      char_sound.play();
    }
  }

  if (INPUT.right.pressed())
  {
    if (selected_map_node < map_nodes.size() - 1)
    {
      selected_map_node += 1;
      char_sound.play();
    }
  }

  size_t tries = map_nodes.size();
  while (tries > 0 && !map_nodes[selected_map_node].discovered)
  {
    if (selected_map_node < map_nodes.size() - 1)
      selected_map_node += 1;
    else
      selected_map_node = 0;

    tries -= 1;
  }
  if (!map_nodes[selected_map_node].discovered)
    selected_map_node = 0;

  if (!has_messages())
  {
    if (action_pressed)
    {
      action_pressed = false;
      printf("Selected node: %zu\n", selected_map_node);

      if (selected_map_node < map_nodes.size())
      {
        auto &n = map_nodes[selected_map_node];
        if (n.occupied || n.completed)
          return;
        load_selected_level();
        select_sound.play();
      }
    }
  }
}

void Game::load_selected_level()
{
  level.reset_player_position = true;

  auto &n = map_nodes[selected_map_node];

  if (!n.discovered && n.name == map_nodes.front().name)
    n.discovered = true;

  if (!n.discovered)
  {
    selected_map_node = 0;
    return;
  }

  for (auto &n : map_nodes)
    n.occupied = false;

  up2_sound.play();
  n.occupied = true;

  std::string level_name = n.name;
  std::replace(level_name.begin(), level_name.end(), ' ', '_');
  level.load(level_name);

  auto &manager = Manager::get();
  game->particle_system.get().clear();
  manager.call_init();

  show_map = false;
}

void Game::draw_map()
{
  DrawTexture(map_texture, map_x, map_y, FULLWHITE);

  for (size_t i = 0; i < map_nodes.size(); ++i)
  {
    const auto &n = map_nodes[i];
    Vector2 p     = n.position;
    p.x           = map_x - 100 + p.x;

    const auto &rd = n.draw_radius;
    const auto &rs = n.radius;

    if (i < map_nodes.size() - 1 && n.discovered)
    {
      const auto &n2 = map_nodes[i + 1];
      if (n2.discovered)
      {
        Vector2 n2p = n2.position;
        n2p.x       = map_x - 100 + n2p.x;
        DrawLineV(p, n2p, PALETTE_WHITE);
        DrawLine(p.x, p.y - 1, n2p.x, n2p.y - 1, PALETTE_BLACK);
      }
    }

    if (n.discovered)
    {
      if (n.occupied)
      {
        DrawCircleV(p, 6, PALETTE_RED);
        DrawCircleLinesV(p, rd + 1, PALETTE_BLACK);
      }

      if (selected_map_node == i)
      {
        DrawCircleLinesV(p, rd, PALETTE[tick() / 8 % 8]);
        DrawCircleLinesV(p, rd + 1, PALETTE[tick() / 8 % 8]);
        DrawCircleLinesV(p, rd + 2, PALETTE[tick() / 8 % 8]);
      }

      DrawCircleLinesV(p, rd - 1, PALETTE_WHITE);

      DrawCircleLinesV(p, rs + 1, PALETTE_BLACK);
      DrawCircleLinesV(p, rs, PALETTE_GREEN);
    }
  }

  if (show_map && selected_map_node < map_nodes.size())
  {
    const auto &n  = map_nodes[selected_map_node];
    const float tx = map_x - 100 + 41;
    float ty       = map_y;

    int connections    = 0;
    int completed      = 0;
    float line_spacing = 10.0f;
    for (const auto &n : map_nodes)
    {
      if (n.occupied)
      {
        DrawTextEx(font, "Current area", Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_GRAY);
        DrawTextEx(font, "Current area", Vector2{ tx, ty }, font_size, font_spacing, PALETTE_WHITE);
        ty += line_spacing;

        DrawTextEx(font, n.name.c_str(), Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_RED);
        DrawTextEx(font, n.name.c_str(), Vector2{ tx, ty }, font_size, font_spacing, PALETTE_WHITE);
        ty += line_spacing;
      }

      if (n.discovered)
        connections += 1;

      if (n.completed)
        completed += 1;
    }

    if (connections > 1)
    {
      ty += line_spacing;
      DrawTextEx(font, "Connected: ", Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_GRAY);
      DrawTextEx(font, "Connected: ", Vector2{ tx, ty }, font_size, font_spacing, PALETTE_WHITE);
      ty += line_spacing;

      DrawTextEx(
        font, TextFormat("%d areas", connections), Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_GRAY);
      DrawTextEx(font, TextFormat("%d areas", connections), Vector2{ tx, ty }, font_size, font_spacing, PALETTE_YELLOW);

      ty += line_spacing;
    }

    const double completed_percent = static_cast<double>(completed) / map_nodes.size() * 100.0;
    ty += line_spacing;
    DrawTextEx(font, "Completed: ", Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_GRAY);
    DrawTextEx(font, "Completed: ", Vector2{ tx, ty }, font_size, font_spacing, PALETTE_WHITE);
    ty += line_spacing;
    DrawTextEx(
      font, TextFormat("%.0f%%", completed_percent), Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_GRAY);
    DrawTextEx(font, TextFormat("%.0f%%", completed_percent), Vector2{ tx, ty }, font_size, font_spacing, PALETTE_BLUE);
    ty += line_spacing;

    const double connected_percent = static_cast<double>(connections) / map_nodes.size() * 100.0;
    ty += line_spacing;
    DrawTextEx(font, "Connectivity: ", Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_GRAY);
    DrawTextEx(font, "Connectivity: ", Vector2{ tx, ty }, font_size, font_spacing, PALETTE_WHITE);
    ty += line_spacing;
    DrawTextEx(
      font, TextFormat("%.0f%%", connected_percent), Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_GRAY);
    DrawTextEx(
      font, TextFormat("%.0f%%", connected_percent), Vector2{ tx, ty }, font_size, font_spacing, PALETTE_YELLOW);

    if (!has_messages())
    {
      size_t last_discovered_node_idx = 0;
      for (size_t i = 0; i < map_nodes.size(); ++i)
      {
        if (map_nodes[i].discovered)
          last_discovered_node_idx = i;
      }

      const int canvas_w = 320;
      const int canvas_h = 180;
      const int border   = 20;
      int dialog_w       = canvas_w - border * 4;
      int dialog_h       = 40;
      int dialog_x       = canvas_w / 2 - dialog_w / 2;
      int dialog_y       = canvas_h - 30 - border;

      std::string level_name(n.name);
      auto text      = TextFormat("%s %s %s",
                             selected_map_node == 0 ? "" : "<",
                             level_name.c_str(),
                             selected_map_node == last_discovered_node_idx ? "" : ">");
      auto text_size = MeasureTextEx(font, text, font_size, font_spacing);
      float tx       = roundf(dialog_x + dialog_w / 2.0f - text_size.x / 2.0f);
      float ty       = roundf(dialog_y + dialog_h / 2.0f - 16.0f);

      DrawTextEx(font, text, Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_BLACK);
      DrawTextEx(font, text, Vector2{ tx, ty }, font_size, font_spacing, PALETTE_WHITE);

      ty += line_spacing;

      text = "Embark";

      if (n.completed)
        text = "Completed";

      if (n.occupied)
        text = "Occupied";

      text_size = MeasureTextEx(font, text, font_size, font_spacing);
      tx        = roundf(dialog_x + dialog_w / 2.0f - text_size.x / 2.0f);

      if (!n.occupied)
        DrawTextEx(font, text, Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE[tick() / 8 % 8]);
      else
        DrawTextEx(font, text, Vector2{ tx, ty + 1.0f }, font_size, font_spacing, PALETTE_RED);

      DrawTextEx(font, text, { tx, ty }, font_size, font_spacing, PALETTE_WHITE);
    }
  }
}
