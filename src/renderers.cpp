#include "renderers.hpp"

#include "game.hpp"
#include "manager.hpp"
#include "sprite.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(SpriteRenderer);
REGISTER_COMPONENT(TileRenderer);

COMPONENT_TEMPLATE(SpriteRenderer);
COMPONENT_TEMPLATE(TileRenderer);

void SpriteInterpolated::render()
{
  if (!visible)
    return;

  if (std::isnan(previous_x) || std::isnan(previous_y))
  {
    previous_x = x;
    previous_y = y;
  }

  if (fabs(previous_x - x) > 8.0f || fabs(previous_y - y) > 8.0f)
  {
    previous_x = x;
    previous_y = y;
  }

  const auto &[draw_x, draw_y] = lerp(previous_x, previous_y, x, y, Game::frame_progress());

  sprite.position.x = draw_x;
  sprite.position.y = draw_y;
  sprite.draw();

  previous_x = draw_x;
  previous_y = draw_y;
}

void SpriteRenderer::render()
{
  sprite_interpolated.render();
}

void TileRenderer::render()
{
  if (!visible)
    return;

  if (fabs(previous_x - x) > 4.0f || fabs(previous_y - y) > 4.0f)
  {
    previous_x = x;
    previous_y = y;
  }

  const auto &[draw_x, draw_y] = lerp(previous_x, previous_y, x, y, Game::frame_progress());

  const auto rect = source();
  TileRenderer::render(sprite, rect, { draw_x, draw_y });

  previous_x = x;
  previous_y = y;
}

inline void TileRenderer::render(Sprite &sprite, Rectangle source, Vector2 position)
{
  const auto &texture = sprite.get_texture();
  DrawTexturePro(texture,
                 source,
                 { position.x, position.y, source.width, source.height },
                 { source.width / 2.0f, source.height / 2.0f },
                 0.0f,
                 WHITE);
}
