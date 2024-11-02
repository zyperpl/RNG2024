#include "sprite.hpp"

#define _USE_MATH_DEFINES
#include <cassert>
#include <chrono>
#include <filesystem>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
#endif

#if defined(__clang__) && !defined(__GNUC__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wshadow"
  #pragma clang diagnostic ignored "-Wunused-variable"
  #pragma clang diagnostic ignored "-Wunused-function"
  #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

#if defined(__GNUC__) && !defined(__clang__)
  #pragma GCC diagnostic pop
#endif
#if defined(__clang__) && !defined(__GNUC__)
  #pragma clang diagnostic pop
#endif

#include "utils.hpp"

template<typename T>
struct CachedResource
{
  CachedResource(const CachedResource &)            = delete;
  CachedResource &operator=(const CachedResource &) = delete;
  CachedResource(CachedResource &&)                 = delete;
  CachedResource &operator=(CachedResource &&)      = delete;

  static inline std::unordered_map<std::string, std::shared_ptr<CachedResource>> cache;
  static inline bool is_used(const std::string &path)
  {
    if (cache.contains(path))
      return cache.at(path)->use_count > 0;

    return false;
  }

  static inline T use(const std::string &path)
  {
    cache[path]->use_count++;
    return cache[path]->data;
  }
  static inline T add(const std::string &path, T data)
  {
    cache[path] = std::shared_ptr<CachedResource>(new CachedResource(data), destruct);
    cache[path]->use_count++;
    return data;
  }

  static inline void free(const std::string &path)
  {
    if (cache.contains(path))
    {
      cache[path]->use_count--;
      if (cache[path]->use_count <= 0)
        cache.erase(path);
    }
  }

private:
  explicit inline CachedResource(T data)
    : data{ data }
  {
  }

  static inline void destruct(CachedResource *res)
  {
    delete res;
  }

  T data;
  uint64_t use_count{ 0 };
};

using CachedTexture = struct CachedResource<Texture2D>;

template<>
inline void CachedTexture::destruct(CachedTexture *res)
{
  UnloadTexture(res->data);
  delete res;
}

using CachedAse = struct CachedResource<ase_t *>;

template<>
inline void CachedAse::destruct(CachedAse *res)
{
  cute_aseprite_free(res->data);
  delete res;
}

Sprite::Sprite(const std::string &file_path, std::string tag_name)
  : path{ file_path }
{
  if (path.ends_with(".aseprite") || path.ends_with(".ase"))
  {
    load_texture_with_animation();
    set_tag(tag_name);
  }
  else
  {
    if (CachedTexture::is_used(file_path))
      texture = CachedTexture::use(file_path);
    else
      texture = CachedTexture::add(path, LoadTexture(std::string(path).c_str()));
  }

  assert(IsTextureValid(texture));
}

Sprite::~Sprite()
{
  CachedAse::free(path);
  CachedTexture::free(path);
}

void Sprite::load_texture_with_animation()
{
  ase_t *ase{ nullptr };

  if (CachedAse::is_used(path))
  {
    ase = CachedAse::use(path);
  }
  else
  {
    ase = cute_aseprite_load_from_file(path.data(), nullptr);
    if (!ase || ase->w <= 0 || ase->h <= 0)
    {
      TraceLog(LOG_ERROR, "Cannot load \"ase\" file \"%s\"", path.data());
      return;
    }

    CachedAse::add(path, ase);
  }

  if (CachedTexture::is_used(path))
  {
    texture = CachedTexture::use(path);
  }
  else
  {
    Image image = GenImageColor(ase->w * ase->frame_count, ase->h, BLANK);

    for (int i = 0; i < ase->frame_count; i++)
    {
      const ase_frame_t *frame{ ase->frames + i };
      const Rectangle dest{
        static_cast<float>(i * ase->w), 0.0f, static_cast<float>(ase->w), static_cast<float>(ase->h)
      };
      const Rectangle src{ 0.0f, 0.0f, static_cast<float>(ase->w), static_cast<float>(ase->h) };
      const Image frameImage{ .data    = frame->pixels,
                              .width   = ase->w,
                              .height  = ase->h,
                              .mipmaps = 1,
                              .format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
      ImageDraw(&image, frameImage, src, dest, FULLWHITE);
    }
    texture = CachedTexture::add(path, LoadTextureFromImage(image));
    UnloadImage(image);
  }

  assert(ase && "Ase file is not loaded");
  assert(ase->w > 0 && ase->w < std::numeric_limits<decltype(frame_width)>::max());
  frame_width = ase->w;
  assert(ase->h > 0 && ase->h < std::numeric_limits<decltype(frame_height)>::max());
  frame_height = ase->h;
  assert(ase->frame_count < std::numeric_limits<decltype(frame_count)>::max());
  frame_count = ase->frame_count;

  for (int i = 0; i < ase->frame_count; ++i)
  {
    const ase_frame_t *frame{ ase->frames + i };
    frame_durations.push_back(frame->duration_milliseconds);
  }

  assert(ase->frame_count < std::numeric_limits<int8_t>::max() - 1);
  if (ase->tag_count > 0 && ase->frame_count > 0)
  {
    TraceLog(LOG_INFO, "Sprite(%s, %d frames) has %d tags:", path.data(), frame_count, ase->tag_count);
    for (int i = 0; i < ase->tag_count; ++i)
    {
      const auto &atag = ase->tags[i];
      TraceLog(
        LOG_INFO, "    > AnimationTag (%d) \"%s\", frames: %d - %d", i, atag.name, atag.from_frame, atag.to_frame);

      assert(atag.from_frame < std::numeric_limits<uint8_t>::max());
      assert(atag.to_frame < std::numeric_limits<uint8_t>::max());

      tags.insert(std::make_pair(
        atag.name, AnimationTag{ static_cast<uint8_t>(atag.from_frame), static_cast<uint8_t>(atag.to_frame) }));
    }
  }

  default_tag.start_frame = 0;
  default_tag.end_frame   = frame_count - 1;
}

const Texture2D &Sprite::get_texture() const
{
#if defined(DEBUG)
  assert(IsTextureValid(texture));
  if (IsKeyPressed(KEY_F7) && std::filesystem::exists(path) && !is_file_written(path))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CachedTexture::free(path);
    CachedTexture::free(path);
    CachedTexture::free(path);
    CachedTexture::free(path);
    Sprite sprite(path);
    texture = CachedTexture::use(path);
  }
#endif
  return texture;
}

size_t Sprite::get_width() const
{
  if (frame_width <= 0)
    return texture.width;

  return frame_width;
}

size_t Sprite::get_height() const
{
  if (frame_height <= 0)
    return texture.height;

  return frame_height;
}

void Sprite::set_centered()
{
  const float origin_x = fabs(static_cast<float>(scale.x * get_width())) / 2.0f;
  const float origin_y = fabs(static_cast<float>(scale.y * get_height())) / 2.0f;
  origin               = Vector2{ std::floor(origin_x), std::floor(origin_y) };
}

Rectangle Sprite::get_source_rect() const
{
  const float w{ static_cast<float>(get_width()) };
  const float h{ static_cast<float>(get_height()) };
  const float h_flip{ scale.x > 0.0f ? 1.0f : -1.0f };
  const float v_flip{ scale.y > 0.0f ? 1.0f : -1.0f };

  return Rectangle{ source_offset.x + static_cast<float>(frame_index) * w, source_offset.y, h_flip * w, v_flip * h };
}

Rectangle Sprite::get_destination_rect() const
{
  const float sprite_w{ static_cast<float>(get_width()) };
  const float sprite_h{ static_cast<float>(get_height()) };

  return Rectangle{ std::roundf(position.x + offset.x),
                    std::roundf(position.y + offset.y),
                    sprite_w * fabsf(scale.x),
                    sprite_h * fabsf(scale.y) };
}

void Sprite::draw() const noexcept
{
  DrawTexturePro(get_texture(), get_source_rect(), get_destination_rect(), origin, rotation, tint);
}

void Sprite::reset_animation()
{
  frame_index = tag.start_frame;
  frame_timer = 0;
}

void Sprite::set_frame(int frame)
{
  if (frame >= get_frame_count())
    frame = get_frame_count() - 1;
  if (frame < 0)
    frame = 0;

  frame_index = frame;
  frame_timer = 0;
}

void Sprite::set_frame_relative(int frame)
{
  set_frame(frame + tag.start_frame);
}

int Sprite::get_frame() const
{
  return frame_index;
}

int Sprite::get_frame_count() const
{
  return frame_count;
}

auto current_time_ms()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

bool Sprite::should_advance_frame()
{
  const auto time_since_epoch_ms = current_time_ms();
  const auto time_difference_ms  = last_time_ms != time_since_epoch_ms ? (time_since_epoch_ms - last_time_ms) : 0;
  last_time_ms                   = time_since_epoch_ms;

  if (tag.end_frame == tag.start_frame)
  {
    frame_index = tag.start_frame;
    frame_timer = 0;
    return false;
  }

  if (frame_count <= 1) [[unlikely]]
    return false;

  const auto frame_array_index = frame_index;
  assert(frame_array_index < frame_count);
  assert(frame_array_index >= 0);

  if (frame_index < 0)
  {
    frame_index = frame_count > 0 ? frame_count - 1 : 0;
    return false;
  }
  if (frame_index >= frame_count)
  {
    frame_index = 0;
    return false;
  }
  const auto frame_duration =
    static_cast<size_t>(frame_array_index) < frame_durations.size() ? frame_durations[frame_array_index] : 100;
  frame_timer += time_difference_ms;

  if (frame_timer >= frame_duration)
  {
    frame_timer -= frame_duration;
    if (frame_timer >= frame_duration)
      frame_timer = 0;

    return true;
  }

  return false;
}

void Sprite::animate(int step)
{
  if (frame_count <= 1)
    return;

  if (should_advance_frame())
  {
    frame_index += step;
    if (frame_index > tag.end_frame || frame_index >= frame_count)
      frame_index = tag.start_frame;

    if (frame_index < tag.start_frame || frame_index < 0)
      frame_index = tag.end_frame;
  }
}

void Sprite::set_tag(const std::string &tag_name)
{
  if (!tag_name.empty())
  {
    auto tags_iterator = tags.find(tag_name);
    if (tags_iterator != tags.end())
    {
      if (tag != tags_iterator->second)
      {
        tag = tags_iterator->second;
        reset_animation();
      }
    }
    else
    {
#if defined(DEBUG)
      TraceLog(LOG_ERROR, "Sprite(%s) has no tag named \"%s\"", path.data(), tag_name.data());
      assert(tags_iterator != tags.end());
#endif
    }
  }
  else
  {
    if (tag != default_tag)
    {
      reset_animation();
      tag = default_tag;
    }
  }
}
