#include "manager.hpp"

#include "utils.hpp"

Manager *manager_instance{ nullptr };

Manager &Manager::get()
{
  if (!manager_instance)
  {
    void *manager_ptr = allocate_manager(std::alignment_of_v<Manager>, sizeof(Manager));
    if (*static_cast<int *>(manager_ptr) == 0)
    {
      manager_instance = new (manager_ptr) Manager();
      printf("Manager created at %p\n", (void *)manager_instance);
    }
    else
      manager_instance = static_cast<Manager *>(manager_ptr);
  }

  return *manager_instance;
}

void Manager::destroy_entity(Entity entity)
{
  entity_destroy_queue.push(entity);
}

void Manager::unregister_all()
{
  for (auto &[_, container] : component_containers)
  {
    if (container.valid)
      container.uninitialize();
  }
}

void Manager::destroy()
{
  for (auto &[_, container] : component_containers)
  {
    if (container.valid)
      std::free(container.manager);
  }
}

Entity Manager::EntityContainer::create()
{
  Entity entity = previous_entity_id + 1;
  assert(entities_count < entities.size() && "Entity container is full");

  entities[entities_count] = entity;
  entities_count += 1;

  previous_entity_id = entity;
  return entity;
}
