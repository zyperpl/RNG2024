#include "particles.hpp"

#include <cassert>

#include "utils.hpp"

REGISTER_COMPONENT(ParticleSystem);
COMPONENT_TEMPLATE(ParticleSystem);

ParticleBuilder &ParticleBuilder::size(float size)
{
  part.size     = size;
  part.size_min = size;
  part.size_max = size;
  return *this;
}

ParticleBuilder &ParticleBuilder::size(float size_min, float size_max, float size_incr)
{
  part.size      = size_min;
  part.size_min  = size_min;
  part.size_max  = size_max;
  part.size_incr = size_incr;
  return *this;
}

ParticleBuilder &ParticleBuilder::life(int life_min, int life_max)
{
  part.life       = GetRandomValue(life_min, life_max);
  part.life_min   = life_min;
  part.life_max   = life_max;
  part.start_life = part.life;
  return *this;
}

ParticleBuilder &ParticleBuilder::alpha(float alpha_beg, float alpha_mid, float alpha_end)
{
  part.alpha  = alpha_beg;
  part.alpha1 = alpha_beg;
  part.alpha2 = alpha_mid;
  part.alpha3 = alpha_end;
  return *this;
}

ParticleBuilder &ParticleBuilder::alpha(float alpha_beg, float alpha_end)
{
  part.alpha  = alpha_beg;
  part.alpha1 = alpha_beg;
  part.alpha2 = (alpha_beg + alpha_end) / 2.0f;
  part.alpha3 = alpha_end;
  return *this;
}

ParticleBuilder &ParticleBuilder::alpha(float alpha)
{
  part.alpha  = alpha;
  part.alpha1 = alpha;
  part.alpha2 = alpha;
  part.alpha3 = alpha;
  return *this;
}

ParticleBuilder &ParticleBuilder::color(Color color_beg, Color color_mid, Color color_end)
{
  part.color  = color_beg;
  part.color1 = color_beg;
  part.color2 = color_mid;
  part.color3 = color_end;
  return *this;
}

ParticleBuilder &ParticleBuilder::color(Color color_beg, Color color_end)
{
  part.color  = color_beg;
  part.color1 = color_beg;
  part.color2 = ColorAlphaBlend(part.color1, ColorAlpha(part.color2, 0.5f), part.color2);
  part.color3 = color_end;
  return *this;
}

ParticleBuilder &ParticleBuilder::color(Color color)
{
  part.color  = color;
  part.color1 = color;
  part.color2 = color;
  part.color3 = color;
  return *this;
}

ParticleBuilder &ParticleBuilder::gravity(float gravity)
{
  part.gravity = gravity;
  return *this;
}

ParticleBuilder &ParticleBuilder::velocity(Vector2 v)
{
  part.v = v;
  return *this;
}

ParticleBuilder &ParticleBuilder::velocity(Vector2 v_min, Vector2 v_max)
{
  part.v_min = v_min;
  part.v_max = v_max;
  return *this;
}

ParticleBuilder &ParticleBuilder::velocity(Vector2 v, Vector2 v_min, Vector2 v_max)
{
  part.v     = v;
  part.v_min = v_min;
  part.v_max = v_max;
  return *this;
}

ParticleBuilder &ParticleBuilder::sprite(const std::string &filename)
{
  part.sprite_id     = system.sprite_id(filename);
  const auto &sprite = system.get_sprite(part.sprite_id);
  part.frame_min     = 0;
  part.frame_max     = sprite.get_frame_count();
  part.frame         = GetRandomValue(0, part.frame_max - 1);
  return *this;
}

ParticleBuilder &ParticleBuilder::sprite(const std::string &filename, float frame_incr)
{
  part.sprite_id  = system.sprite_id(filename);
  part.frame      = 0.0f;
  part.frame_min  = 0;
  part.frame_max  = 0;
  part.frame_incr = frame_incr;
  return *this;
}

Particle ParticleBuilder::build()
{
  return part;
}

void ParticleSystem::add_particle(int x, int y, const Particle &type)
{
  Particle part = type;

  part.x = x;
  part.y = y;

  part.life       = GetRandomValue(part.life_min, part.life_max);
  part.start_life = part.life;

  part.size = part.size_min + (part.size_max - type.size_min) * randf();

  part.v.x += part.v_min.x + (part.v_max.x - part.v_min.x) * randf();
  part.v.y += part.v_min.y + (part.v_max.y - part.v_min.y) * randf();

  part.frame = static_cast<float>(GetRandomValue(part.frame_min, part.frame_max - 1));

  particles.push_back(part);
}

void ParticleSystem::add_particles(int x, int y, const Particle &type, int count)
{
  for (int i = 0; i < count; i++)
  {
    add_particle(x, y, type);
  }
}

void ParticleSystem::update()
{
  for (auto &particle : particles)
  {
    particle.v.y += particle.gravity;

    particle.x += particle.v.x;
    particle.y += particle.v.y;

    particle.v.x += particle.v_inc.x;
    particle.v.y += particle.v_inc.y;

    particle.size += particle.size_incr;
    if (particle.size < 0.0f)
      particle.size = 0.0f;

    particle.frame += particle.frame_incr;

    particle.life -= 1;

    const float life_percent =
      static_cast<float>(particle.start_life - particle.life) / static_cast<float>(particle.start_life);
    if (life_percent < 0.5f)
    {
      particle.alpha = particle.alpha1 + ((particle.alpha2 - particle.alpha1) * life_percent);
      particle.color.r =
        static_cast<unsigned char>(particle.color1.r + ((particle.color2.r - particle.color1.r) * life_percent));
      particle.color.g =
        static_cast<unsigned char>(particle.color1.g + ((particle.color2.g - particle.color1.g) * life_percent));
      particle.color.b =
        static_cast<unsigned char>(particle.color1.b + ((particle.color2.b - particle.color1.b) * life_percent));
    }
    else
    {
      particle.alpha = particle.alpha2 + ((particle.alpha3 - particle.alpha2) * life_percent);
      particle.color.r =
        static_cast<unsigned char>(particle.color2.r + ((particle.color3.r - particle.color2.r) * life_percent));
      particle.color.g =
        static_cast<unsigned char>(particle.color2.g + ((particle.color3.g - particle.color2.g) * life_percent));
      particle.color.b =
        static_cast<unsigned char>(particle.color2.b + ((particle.color3.b - particle.color2.b) * life_percent));
    }
  }

  std::erase_if(particles, [](const Particle &particle) { return particle.life <= 0; });
}

void ParticleSystem::render()
{
  if (!visible)
    return;

  // TODO: implement interpolation

  for (const auto &particle : particles)
  {
    const float alpha = roundf(particle.alpha * 4.0f) / 4.0f;
    if (particle.sprite_id != std::numeric_limits<size_t>::max())
    {
      assert(particle.sprite_id < sprites.size());
      auto &sprite = sprites[particle.sprite_id];

      const int frame = static_cast<int>(roundf(particle.frame));

      if (frame < 0 || frame >= sprite.get_frame_count())
        continue;

      sprite.position = Vector2{ particle.x, particle.y };
      sprite.tint     = ColorAlpha(particle.color, alpha);
      sprite.set_frame(frame);
      sprite.scale.x = particle.size;
      sprite.scale.y = particle.size;
      sprite.set_centered();
      sprite.draw();
    }
    else
    {
      if (particle.size > 1.0f)
      {
        DrawCircle(particle.x, particle.y, particle.size, ColorAlpha(particle.color, alpha));
      }
      else
      {
        DrawPixel(particle.x, particle.y, ColorAlpha(particle.color, alpha));
      }
    }
  }
}

size_t ParticleSystem::sprite_id(const std::string &filename)
{
  if (sprite_ids.contains(filename))
    return sprite_ids[filename];

  return add_sprite(filename);
}

const Sprite &ParticleSystem::get_sprite(size_t id) const
{
  assert(id < sprites.size());
  return sprites[id];
}

Sprite &ParticleSystem::get_sprite(size_t id)
{
  assert(id < sprites.size());
  return sprites[id];
}

size_t ParticleSystem::add_sprite(const std::string &filename)
{
  if (sprite_ids.contains(filename))
    return sprite_ids[filename];

  const size_t index = sprites.size();
  const char MOCK_SEPARATOR = ':';
  if (filename.find(MOCK_SEPARATOR) != std::string::npos)
  {
    const auto filename_substr = filename.substr(0, filename.find(MOCK_SEPARATOR));
    printf("Mocking sprite: %s as %s\n", filename_substr.c_str(), filename.c_str());
    sprites.emplace_back(filename_substr);
  }
  else
  {
    sprites.emplace_back(filename);
  }
  sprite_ids.insert({ filename, index });
  return index;
}

void ParticleSystem::clear()
{
  particles.clear();
}
