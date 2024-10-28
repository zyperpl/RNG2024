#pragma once

#include "component.hpp"
#include "level.hpp"
#include "level_definitions.hpp"
#include "physics.hpp"
#include "utils.hpp"

struct Block
{
  COMPONENT(Block);

  Block(const Level::Entity &entity)
    : x{ entity.position.x }
    , y{ entity.position.y }
    , w{ entity.size.w }
    , h{ entity.size.h }
  {
  }

  Block(const Level::Tile &tile)
    : x{ tile.position.x }
    , y{ tile.position.y }
    , w{ tile.size.w }
    , h{ tile.size.h }
  {
  }

  void init()
  {
    auto &physics       = add_component(entity, Physics()).get();
    physics.x           = x;
    physics.y           = y;
    physics.mask.width  = w;
    physics.mask.height = h;
    physics.solid       = true;
    physics.movable     = false;
    physics.do_update   = false;
  }

private:
  int x{ 0 };
  int y{ 0 };
  int w{ 0 };
  int h{ 0 };
};
