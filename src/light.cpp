#include "light.hpp"

#include "level.hpp"
#include "utils.hpp"

REGISTER_COMPONENT(Light);
COMPONENT_TEMPLATE(Light);
REGISTER_LEVEL_ENTITY(Light);

Light::Light(const Level::Entity &entity)
{
  start_x = entity.position.x + entity.size.w / 2;
  start_y = entity.position.y + entity.size.h / 2;

  Level::read_field(entity.fields, "Size", size);
  Level::read_field(entity.fields, "Intensity", intensity);
}

void Light::init()
{
  x = start_x;
  y = start_y;
}
