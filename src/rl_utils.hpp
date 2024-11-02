#pragma once

#include <cassert>
#include <string>
#include <string_view>

#include <raylib.h>
#include <raymath.h>

namespace NRL
{
struct FragShader
{
  [[nodiscard]] FragShader(std::string_view path)
    : path{ path }
  {
    load(path);
  }

  ~FragShader()
  {
    unload();
  }

  FragShader(const FragShader &)            = delete;
  FragShader &operator=(const FragShader &) = delete;
  FragShader(FragShader &&)                 = delete;
  FragShader &operator=(FragShader &&)      = delete;

  inline void load(std::string_view path)
  {
    this->path = path;
    shader     = LoadShader(0, path.data());
    assert(IsShaderValid(shader));
  }

  inline bool valid() const
  {
    return IsShaderValid(shader);
  }

  inline bool enable() const
  {
#if defined(DEBUG)
    assert(enabled_shader == 0 && "Shader already enabled");
    enabled_shader = shader.id;
#endif

    BeginShaderMode(shader);
    return valid();
  }

  inline void disable() const
  {
#if defined(DEBUG)
    assert(enabled_shader == shader.id && "Shader not enabled");
    enabled_shader = 0;
#endif

    EndShaderMode();
  }

  inline void reload()
  {
    unload();
    load(path);
  }

  inline void unload()
  {
    if (IsShaderValid(shader))
      UnloadShader(shader);
  }

  ::Shader shader{ 0 };
  std::string path;

#if defined(DEBUG)
  static inline unsigned int enabled_shader{ 0 };
#endif
};

struct RenderTexture
{
  enum class Smooth
  {
    No,
    Yes
  };

  [[nodiscard]] inline RenderTexture(int width, int height, Smooth smooth = Smooth::Yes)
    : value{ LoadRenderTexture(width, height) }
  {
    assert(valid() && "Failed to create render texture");

    if (smooth == Smooth::Yes)
      apply_smooth();
  }

  RenderTexture(const RenderTexture &)            = delete;
  RenderTexture &operator=(const RenderTexture &) = delete;
  RenderTexture(RenderTexture &&)                 = delete;
  RenderTexture &operator=(RenderTexture &&)      = delete;

  ~RenderTexture()
  {
    UnloadRenderTexture(value);
  }

  inline void resize(int width, int height)
  {
    if (width == value.texture.width && height == value.texture.height)
      return;

    width  = std::max(1, width);
    height = std::max(1, height);

    UnloadRenderTexture(value);
    value = LoadRenderTexture(width, height);

    SetTextureWrap(value.texture, TEXTURE_WRAP_CLAMP);

    if (smooth == Smooth::Yes)
      apply_smooth();
  }

  inline void apply_smooth()
  {
    smooth = Smooth::Yes;
    SetTextureFilter(value.texture, TEXTURE_FILTER_BILINEAR);
  }

  inline bool valid() const
  {
    return IsRenderTextureValid(value);
  }

  inline bool enable() const
  {
    if (!valid())
      return false;

    BeginTextureMode(value);
    return true;
  }

  inline void disable() const
  {
    EndTextureMode();
  }

  inline void draw(const Rectangle &destination)
  {
    if (!valid())
      return;

    DrawTexturePro(value.texture, flip ? rect_flipped() : rect(), destination, Vector2Zero(), 0, FULLWHITE);
  }

  inline Rectangle rect() const
  {
    return Rectangle{ 0, 0, static_cast<float>(value.texture.width), static_cast<float>(value.texture.height) };
  }

  inline Rectangle rect_flipped() const
  {
    return Rectangle{ 0, 0, static_cast<float>(value.texture.width), -static_cast<float>(value.texture.height) };
  }

  bool flip{ true };
  ::RenderTexture value{ 0 };
  Smooth smooth{ Smooth::Yes };
};

struct ScreenEffect
{
  ScreenEffect(std::string_view frag_path)
    : shader{ frag_path }
  {
  }

  inline void reload()
  {
    shader.reload();
  }

  inline void resize(int width, int height)
  {
    assert(render_texture.valid() && "Render texture is not valid");
    render_texture.resize(width, height);
  }

  inline bool enable()
  {
    assert(shader.valid() && "Shader is not valid");
    if (shader.enable())
    {
      assert(render_texture.valid() && "Render texture is not valid");
      if (render_texture.enable())
        return true;

      shader.disable();
    }
    return false;
  }

  inline void disable()
  {
    render_texture.disable();
    shader.disable();
  }

  inline void draw_texture(Rectangle dest)
  {
    render_texture.draw(dest);
  }

  NRL::FragShader shader;
  NRL::RenderTexture render_texture{ 8, 8, NRL::RenderTexture::Smooth::No };
};

} // namespace NRL

[[nodiscard]] static inline constexpr bool operator==(const Color &lhs, const Color &rhs)
{
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
