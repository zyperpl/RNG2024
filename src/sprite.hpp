#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <raylib.h>
#include <raymath.h>

class Sprite final
{
public:
  struct AnimationTag
  {
    uint8_t start_frame{ 0 };
    uint8_t end_frame{ 1 };

    auto operator<=>(const AnimationTag &) const = default;
  };
  typedef std::unordered_map<std::string, AnimationTag> AnimationTags;

  [[nodiscard]] Sprite(const std::string &file_path, std::string tag = {});
  Sprite() = default;
  ~Sprite();

  Sprite(const Sprite &)            = delete;
  Sprite &operator=(const Sprite &) = delete;

  Sprite(Sprite &&)            = default;
  Sprite &operator=(Sprite &&) = default;

  void draw() const noexcept;

  [[nodiscard]] const Texture2D &get_texture() const;
  [[nodiscard]] size_t get_width() const;
  [[nodiscard]] size_t get_height() const;

  [[nodiscard]] Rectangle get_source_rect() const;
  [[nodiscard]] Rectangle get_destination_rect() const;

  void set_frame(int frame);

  // sets frame relative to tag start frame
  void set_frame_relative(int frame);

  [[nodiscard]] int get_frame() const;
  [[nodiscard]] int get_frame_count() const;

  void set_tag(const std::string &tag_name);
  inline void set_animation(const std::string &tag_name)
  {
    set_tag(tag_name);
  }
  [[nodiscard]] bool is_playing_animation(const std::string &tag_name) const
  {
    return tag == tags.at(tag_name);
  }

  void reset_animation();
  void animate(int step = 1);

  const std::string &get_path() const noexcept
  {
    return path;
  }

  void set_centered();

  Vector2 position{ 0.0f, 0.0f };
  Vector2 origin{ 0.0f, 0.0f };
  Vector2 offset{ 0.0f, 0.0f };
  Vector2 source_offset{ 0.0f, 0.0f };
  Vector2 scale{ 1.0f, 1.0f };

  Color tint{ WHITE };
  float rotation{ 0.0f };

  inline bool has_tag(const std::string &tag_name) const
  {
    return tags.find(tag_name) != tags.end();
  }

  inline void set_frame_width(int16_t width)
  {
    frame_width = width;
  }

  inline void set_frame_height(int16_t height)
  {
    frame_height = height;
  }

  inline void set_frame_durations(const std::vector<int32_t> &durations)
  {
    frame_durations = durations;
  }

  inline void set_frame_durations(int32_t duration)
  {
    frame_durations.resize(frame_count);
    std::fill_n(frame_durations.begin(), frame_count, duration);
  }

  inline void set_frame_count(int8_t count)
  {
    frame_count             = count;
    default_tag.start_frame = 0;
    default_tag.end_frame   = count - 1;
    tag                     = default_tag;
  }

protected:
  [[nodiscard]] bool should_advance_frame();

  mutable Texture texture{};
  std::string path{};

private:
  void load_texture_with_animation();

  AnimationTags tags;
  AnimationTag default_tag;

  AnimationTag tag{ 0, 1 };
  int8_t frame_index{ 0 };
  int8_t frame_count{ 0 };
  int16_t frame_width{ 0 };
  int16_t frame_height{ 0 };

  std::vector<int32_t> frame_durations; // in milliseconds
  int64_t frame_timer{ 0 };
  int64_t last_time_ms{ 0 };
};
