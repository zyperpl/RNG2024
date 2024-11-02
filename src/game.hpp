#pragma once

#include <cstddef>
#include <functional>
#include <raylib.h>

#include "component.hpp"
#include "level.hpp"
#include "particles.hpp"
#include "rl_utils.hpp"

extern "C"
{
  void G_create_game();
  void G_destroy_game();
  void G_draw_game(double, RenderTexture &, RenderTexture &);
  void G_reload_game();
  void G_unload_game();
  void G_update_game();
};

constexpr const inline Color PALETTE[8]{ Color{ 0, 0, 0, 255 },       Color{ 85, 65, 95, 255 },
                                         Color{ 100, 105, 100, 255 }, Color{ 80, 140, 215, 255 },
                                         Color{ 100, 185, 100, 255 }, Color{ 215, 115, 85, 255 },
                                         Color{ 230, 200, 110, 255 }, Color{ 220, 245, 255, 255 } };

constexpr const inline Color &PALETTE_BLACK{ PALETTE[0] };
constexpr const inline Color &PALETTE_PURPLE{ PALETTE[1] };
constexpr const inline Color &PALETTE_GRAY{ PALETTE[2] };
constexpr const inline Color &PALETTE_BLUE{ PALETTE[3] };
constexpr const inline Color &PALETTE_GREEN{ PALETTE[4] };
constexpr const inline Color &PALETTE_RED{ PALETTE[5] };
constexpr const inline Color &PALETTE_YELLOW{ PALETTE[6] };
constexpr const inline Color &PALETTE_WHITE{ PALETTE[7] };

struct Game final
{
  Game();

  [[nodiscard]] static double frame_progress();
  static void add_timer(Entity, std::function<void()> &&, size_t frames);
  static void add_particles(int x, int y, const Particle &type, size_t count = 1);
  [[nodiscard]] static ParticleBuilder particle_builder();
  [[nodiscard]] static int64_t level_width();
  [[nodiscard]] static int64_t level_height();
  [[nodiscard]] static std::string_view level_name();
  [[nodiscard]] static size_t tick();
  [[nodiscard]] static size_t frame();
  static void skip_ticks(size_t count);
  [[nodiscard]] static bool on_screen(Rectangle rect);
  static void defer_draw(Entity entity, std::function<void()> &&callback);
  static void queue_message(std::string message, Color color = PALETTE_WHITE);
  static void end_level();

private:
  [[nodiscard]] static Game &get();

  void init();

  struct Values
  {
    double frame_progress{ 0.0 };
  };
  Values values;

  struct Timer
  {
    Entity entity;
    std::function<void()> callback;
    size_t frames;
  };
  std::vector<Timer> timers;
  void update_timers();

  Texture palette_texture;
  void generate_palette_texture();

  size_t ticks{ 0 };
  size_t frames{ 0 };

  size_t skip_ticks_count{ 0 };

  Camera2D camera;
  NRL::RenderTexture render_texture{ 8, 8 };
  NRL::ScreenEffect dither_fx{ "assets/shaders/dither_frag.glsl" };
  Level::Level level;
  ComponentReference<ParticleSystem> particle_system;
  Music music;
  bool mute{ true };
  Font font;
  float font_size{ 18 };
  float font_spacing{ 1 };

  void draw_deferred();
  std::vector<std::pair<Entity, std::function<void()>>> deferred_draws;

  bool action_pressed { false };

  void update_messages();
  void draw_messages();
  [[nodiscard]] bool has_messages() const
  {
    return !messages.empty();
  }
  std::queue<std::pair<std::string, Color>> messages;
  std::pair<std::string, Color> drawn_message;
  bool message_ready{ false };

  struct MapNode
  {
    std::string name;
    Vector2 position;
    float radius{ 30.0f };
    bool occupied{ false };
    bool discovered{ false };
    bool completed { false };
    float draw_radius{ 100.0f };
  };
  std::vector<MapNode> map_nodes{ { "Area Zero", Vector2{ 164, 92 }, 30.0f, true }, { "Habitat", Vector2{ 186, 82 }, 10.0f } };
  void update_map();
  void draw_map();
  bool show_map{ false };
  int map_x = 400;
  int map_y = 6;
  size_t selected_map_node { 0 };
  Texture map_texture{ LoadTexture("assets/map.png") };

  friend void G_create_game();
  friend void G_reload_game();
  friend void G_unload_game();
  friend void G_update_game();
  friend void G_draw_game(double, RenderTexture &, RenderTexture &);

  bool debug_auto_shader_reload{ false };
};
