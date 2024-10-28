#pragma once

#include <cstdio>
#include <random>

#if defined(__linux__)
  #include <dlfcn.h>
  #include <fcntl.h>
  #include <unistd.h>
#elif defined(_WIN32)
  #include <windows.h>
#endif

#include "rl_utils.hpp"
#include <raylib.h>

[[nodiscard]] static inline float randf()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0f, 1.0f);
  return dis(gen);
}

[[nodiscard]] static inline float randf(float max)
{
  assert(max > 0);

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0f, max);
  return dis(gen);
}

[[nodiscard]] static inline float randf(float min, float max)
{
  if (min == max)
    return min;

  if (min > max)
    std::swap(min, max);

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(min, max);
  return dis(gen);
}

[[nodiscard]] static inline int randi(int max)
{
  assert(max > 0);

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, max);
  return dis(gen);
}

[[nodiscard]] static inline int randi(int min, int max)
{
  if (min == max)
    return min;

  if (min > max)
    std::swap(min, max);

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(min, max);
  return dis(gen);
}

[[nodiscard]] static inline Rectangle texture_rect(const Texture &tex)
{
  return { 0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height) };
}

[[nodiscard]] static inline Rectangle texture_rect_flip(const Texture &tex)
{
  return { 0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(-tex.height) };
}

[[nodiscard]] static inline Rectangle target_source(const RenderTexture &target)
{
  return { 0.0f, 0.0f, static_cast<float>(target.texture.width), static_cast<float>(target.texture.height) };
}

[[nodiscard]] static inline Rectangle target_source_flip(const RenderTexture &target)
{
  return { 0.0f, 0.0f, static_cast<float>(target.texture.width), static_cast<float>(-target.texture.height) };
}

[[nodiscard]] inline auto chance(const int percent) -> bool
{
  return GetRandomValue(1, 100) <= percent;
}

constexpr inline auto reduce(auto value, auto amount)
{
  if (value > 0)
    return std::max(static_cast<decltype(value)>(0), value - amount);
  else
    return std::min(static_cast<decltype(value)>(0), value + amount);
}

constexpr inline auto round(const Vector2 &v)
{
  return Vector2{ std::round(v.x), std::round(v.y) };
}

constexpr inline auto inflate(const Rectangle &rect, float amount)
{
  return Rectangle{ rect.x - amount, rect.y - amount, rect.width + amount * 2, rect.height + amount * 2 };
}

#if __cpp_lib_interpolate < 201902L || defined(EMSCRIPTEN)
constexpr inline auto lerp(auto x1, auto x2, auto t)
{
  return x1 + (x2 - x1) * t;
}
#endif

constexpr inline auto lerp(auto x1, auto y1, auto x2, auto y2, auto t) -> std::pair<decltype(x1), decltype(y1)>
{
  return { x1 + (x2 - x1) * t, y1 + (y2 - y1) * t };
}

[[nodiscard]] inline auto is_file_written(const std::string_view &path)
{
#if defined(__linux__)
  int fd = open(path.data(), O_RDONLY);
  if (fd == -1)
    return false;

  struct flock lock;
  lock.l_type   = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start  = 0;
  lock.l_len    = 0;

  bool is_locked = fcntl(fd, F_GETLK, &lock) != -1 && lock.l_type != F_UNLCK;
  close(fd);

  return is_locked;
#else
  return false;
#endif
}

inline void generate_id(size_t &counter, std::string &level_id)
{
  level_id = std::to_string(counter++) + "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[randi(25)];
}

extern void *allocate(size_t size);
extern void deallocate(void *memory);

template<typename T>
T *allocate()
{
  return static_cast<T *>(allocate(sizeof(T)));
}

extern void *allocate_manager(size_t alignment, size_t size);
extern void *allocate_game(size_t alignment, size_t size);
