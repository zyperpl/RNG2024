#include "light.hpp"

#include "utils.hpp"
#include "level.hpp"

REGISTER_COMPONENT(Light);
COMPONENT_TEMPLATE(Light);
REGISTER_LEVEL_ENTITY(Light);

Light::Light(const Level::Entity &entity)
{
  start_x = entity.position.x + entity.size.w / 2;
  start_y = entity.position.y + entity.size.h / 2;

  Level::read_field(entity.fields, "Strength", strength);
}

void Light::init()
{

}

