#pragma once

#include <string>

#include <raylib.h>

struct GameSound
{
  [[nodiscard]] GameSound(const std::string &file_path);
  GameSound() = default;
  ~GameSound();

  GameSound(const GameSound &)            = delete;
  GameSound &operator=(const GameSound &) = delete;

  GameSound(GameSound &&)            = default;
  GameSound &operator=(GameSound &&) = default;

  void play() const noexcept;
  void stop() const noexcept;

  [[nodiscard]] bool is_playing() const noexcept;

  void set_volume(float v)
  {
    volume = v;
  }
  void set_pitch(float p)
  {
    pitch = p;
  }

private:
  std::string path;
  mutable size_t last_index{ 0 };
  float pitch{ 1.0f };
  float volume{ 1.0f };
};
