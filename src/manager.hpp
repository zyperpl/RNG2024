#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <set>
#include <span>
#include <unordered_map>

#include "component.hpp"

constexpr inline size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

using ReferenceIndex = size_t;

template<typename C>
struct ComponentReference;

constexpr inline size_t MAX_COMPONENTS_PER_TYPE = 4096;
constexpr inline Entity ENTITY_START_ID         = 10;
constexpr inline int DEFAULT_DEPTH              = -9;
constexpr inline int NEG_INF_DEPTH              = std::numeric_limits<int>::min();

#define GEN_HAS_FUNCTION_CONCEPT(func)               \
  template<typename C, typename... Args>             \
  concept has_##func = requires(C c, Args... args) { \
    { c.func(args...) } -> std::same_as<void>;       \
  };

#define GEN_HAS_MEMBER_CONCEPT(member)   \
  template<typename C>                   \
  concept has_##member = requires(C c) { \
    { c.member };                        \
  };

GEN_HAS_FUNCTION_CONCEPT(init);
GEN_HAS_FUNCTION_CONCEPT(preupdate);
GEN_HAS_FUNCTION_CONCEPT(update);
GEN_HAS_FUNCTION_CONCEPT(postupdate);
GEN_HAS_FUNCTION_CONCEPT(render);
GEN_HAS_FUNCTION_CONCEPT(collision);
GEN_HAS_FUNCTION_CONCEPT(destroyed);

GEN_HAS_MEMBER_CONCEPT(depth);

struct Manager final
{
  static Manager &get();

  void unregister_all();
  void destroy();

  struct DrawCall
  {
    int depth{ 0 };
    std::function<void()> render;
  };

  template<typename C>
  struct ComponentManager
  {
    using ComponentIndex = size_t;

    void push(Entity entity, C &&component)
    {
      static_assert(sizeof(C) < COMPONENT_PADDING_SIZE);
      static_assert(sizeof(ComponentPadded<C>) == COMPONENT_PADDING_SIZE);
      assert(components_count < components.size());

      component.entity              = entity;
      components[components_count]  = std::move(component);
      init_called[components_count] = false;
      component_indices.push_back(components_count);
      index_components[components_count] = component_indices.size() - 1;

      components_count += 1;
    }

    [[nodiscard]] inline size_t count() const
    {
      return components_count;
    }

    [[nodiscard]] inline C &get(ComponentIndex component_index)
    {
      assert(component_index < components_count);
      return components[component_index].component;
    }

    void remove(ComponentIndex component_index)
    {
      assert(component_index < components_count);

      const auto reference_index         = get_reference_index(component_index);
      component_indices[reference_index] = INVALID_INDEX;

      if (components_count > 1)
      {
        std::swap(components[component_index], components[components_count - 1]);
        std::swap(init_called[component_index], init_called[components_count - 1]);
        const auto swapped_reference_index         = get_reference_index(components_count - 1);
        component_indices[swapped_reference_index] = component_index;
        index_components[component_index]          = swapped_reference_index;
      }

      components_count -= 1;
    }

    void set_init_called(ComponentIndex component_index)
    {
      assert(component_index < components_count);
      init_called[component_index] = true;
    }

    [[nodiscard]] bool was_init_called(ComponentIndex component_index) const
    {
      assert(component_index < components_count);
      return init_called[component_index];
    }

    [[nodiscard]] inline ComponentIndex get_component_index(ReferenceIndex reference_index) const
    {
      assert(reference_index < component_indices.size());
      return component_indices[reference_index];
    }

    [[nodiscard]] inline ReferenceIndex get_reference_index(ComponentIndex component_index) const
    {
      assert(component_index < components_count);
      return index_components[component_index];
    }

  private:
    std::array<ComponentPadded<C>, MAX_COMPONENTS_PER_TYPE> components;
    std::array<bool, MAX_COMPONENTS_PER_TYPE> init_called;
    size_t components_count{ 0 };
    std::vector<ComponentIndex> component_indices;                        // reference index -> component index
    std::array<ReferenceIndex, MAX_COMPONENTS_PER_TYPE> index_components; // component index -> reference index
    friend struct Manager;
  };

private:
  struct ComponentManagerContainer
  {
    template<typename C>
    [[nodiscard]] inline ComponentManager<C> &get_manager() const
    {
      assert(id == C::id() && "Component id mismatch");
      assert(valid && "Component manager not valid");
      return *static_cast<ComponentManager<C> *>(manager);
    }

    std::function<void()> init{ nullptr };
    std::function<void()> preupdate{ nullptr };
    std::function<void()> update{ nullptr };
    std::function<void()> postupdate{ nullptr };
    std::function<void()> render{ nullptr };
    std::function<void(Entity, Entity)> collision{ nullptr };
    std::function<void(Entity)> remove{ nullptr };
    std::function<void(Entity)> destroyed{ nullptr };

  private:
    bool valid{ false };
    ComponentType id{ 0 };
    void *manager{ nullptr };

    inline void uninitialize()
    {
      init       = nullptr;
      preupdate  = nullptr;
      update     = nullptr;
      postupdate = nullptr;
      render     = nullptr;
      collision  = nullptr;
      remove     = nullptr;
      destroyed  = nullptr;
    }

    friend struct Manager;
  };

  struct EntityContainer
  {
    Entity entity{ INVALID_ENTITY };
    std::array<Entity, 4096> entities;
    size_t entities_count{ 0 };

    EntityContainer() = default;

    EntityContainer(const EntityContainer &)            = delete;
    EntityContainer &operator=(const EntityContainer &) = delete;
    EntityContainer(EntityContainer &&)                 = delete;
    EntityContainer &operator=(EntityContainer &&)      = delete;

    [[nodiscard]] Entity create();

    void remove(Entity entity)
    {
      auto it = std::find(std::begin(entities), std::begin(entities) + entities_count, entity);
      if (it == std::end(entities) || it == std::begin(entities) + entities_count)
      {
        // Entity not found
        return;
      }
      std::swap(*it, entities[entities_count - 1]);
      entities_count -= 1;
    }

    [[nodiscard]] bool contains(Entity entity) const
    {
      if (entity == INVALID_ENTITY)
        return false;

      assert(entities_count < entities.size());

      return std::find(std::begin(entities), std::begin(entities) + entities_count, entity) !=
             std::begin(entities) + entities_count;
    }

  private:
    Entity previous_entity_id{ ENTITY_START_ID };
  };

  inline Manager() = default;

  Manager(const Manager &)            = delete;
  Manager &operator=(const Manager &) = delete;
  Manager(Manager &&)                 = delete;
  Manager &operator=(Manager &&)      = delete;

  [[nodiscard]] inline Entity create_entity()
  {
    Entity entity = entity_container.create();
    return entity;
  }

  void destroy_entity(Entity);

  [[nodiscard]] bool entity_exists(Entity entity) const
  {
    return entity_container.contains(entity);
  }

  template<typename C>
  void register_component()
  {
    printf("Registering component: %lu (func: %s) (size: %lu)\n", C::id(), __PRETTY_FUNCTION__, sizeof(C));

    auto &container = component_containers[C::id()];
    if (!container.valid)
    {
      const size_t manager_size = sizeof(ComponentManager<C>);
      container.id              = C::id();
      container.manager         = new (std::malloc(manager_size)) ComponentManager<C>();
      container.valid           = true;
    }

    auto &manager           = Manager::get();
    auto &component_manager = manager.component_containers[C::id()].template get_manager<C>();

    if constexpr (has_init<C>)
    {
      has_new_init   = true;
      container.init = [&]()
      {
        for (size_t i = 0; i < component_manager.count(); i++)
        {
          auto &component = component_manager.get(i);
          if (!component_manager.was_init_called(i))
          {
            component.init();
            component_manager.set_init_called(i);
          }
        }
      };
      init_components.insert(C::id());
    }

    if constexpr (has_preupdate<C>)
    {
      container.preupdate = [&]()
      {
        for (size_t i = 0; i < component_manager.count(); i++)
          component_manager.get(i).preupdate();
      };
      preupdate_components.insert(C::id());
    }

    if constexpr (has_update<C>)
    {
      container.update = [&]()
      {
        for (size_t i = 0; i < component_manager.count(); i++)
          component_manager.get(i).update();
      };
      update_components.insert(C::id());
    }

    if constexpr (has_postupdate<C>)
    {
      container.postupdate = [&]()
      {
        for (size_t i = 0; i < component_manager.count(); i++)
          component_manager.get(i).postupdate();
      };
      postupdate_components.insert(C::id());
    }

    if constexpr (has_render<C>)
    {
      last_draw_call_index = std::numeric_limits<size_t>::max();
      container.render     = [&]()
      {
        for (size_t i = 0; i < component_manager.count(); i++)
        {
          auto &component = component_manager.get(i);
          if constexpr (has_depth<C>)
          {
            draw_calls.emplace_back(DrawCall{ component.depth, [&]() { component.render(); } });
          }
          else
          {
            draw_calls.emplace_back(DrawCall{ DEFAULT_DEPTH, [&]() { component.render(); } });
          }
        }
      };
      render_components.insert(C::id());
    }

    if constexpr (has_collision<C, Entity>)
    {
      container.collision = [&](Entity owner, Entity collider)
      {
        for (size_t i = 0; i < component_manager.count(); i++)
        {
          auto &component = component_manager.get(i);
          if (component.entity == owner)
            component.collision(collider);
        }
      };
      collision_components.insert(C::id());
    }

    container.remove = [&](Entity entity)
    {
      for (size_t i = 0; i < component_manager.count();)
      {
        if (component_manager.get(i).entity == entity)
          component_manager.remove(i);
        else
          i++;
      }
    };

    if constexpr (has_destroyed<C>)
    {
      container.destroyed = [&](Entity entity)
      {
        for (size_t i = 0; i < component_manager.count(); i++)
        {
          auto &component = component_manager.get(i);
          if (component.entity == entity)
            component.destroyed();
        }
      };
      destroyed_components.insert(C::id());
    }
  }

public:
  void call_init()
  {
    while (has_new_init)
    {
      has_new_init = false;

      for (auto &component_id : init_components)
      {
        auto &container = component_containers[component_id];
        if (container.init)
          container.init();
      }
    }
  }

  void call_destroy()
  {
    assert(created && "Manager not created");

    while (!entity_destroy_queue.empty())
    {
      auto entity = entity_destroy_queue.front();
      for (auto &component_type : destroyed_components)
      {
        auto &container = component_containers[component_type];
        if (container.valid && container.destroyed)
          container.destroyed(entity);
      }

      for (auto &[_, container] : component_containers)
      {
        if (container.valid)
          container.remove(entity);
      }

      entity_container.remove(entity);
      entity_destroy_queue.pop();
    }
  }

  void call_render(int until_depth = NEG_INF_DEPTH)
  {
    for (auto &component_id : render_components)
    {
      auto &container = component_containers[component_id];
      if (container.render)
        container.render();
    }

    // Farther objects have higher depth values
    std::stable_sort(std::begin(draw_calls),
                     std::end(draw_calls),
                     [](const DrawCall &a, const DrawCall &b) { return a.depth > b.depth; });

    size_t start_index = last_draw_call_index;
    if (start_index >= draw_calls.size() - 1)
      start_index = 0;

    for (size_t i = start_index; i < draw_calls.size(); i++)
    {
      last_draw_call_index = i;

      const auto &draw_call = draw_calls[i];

      if (draw_call.depth < until_depth)
        break;

      draw_call.render();
    }
    if (until_depth == NEG_INF_DEPTH)
      last_draw_call_index = std::numeric_limits<size_t>::max();

    draw_calls.clear();
  }

private:
  template<typename C>
  ComponentReference<C> add_component(Entity entity, C &&component)
  {
    assert(entity != INVALID_ENTITY && "Invalid entity");
    auto &container = component_containers[C::id()];
    assert(container.valid && "Component manager does not exist");
    component.entity        = entity;
    auto &component_manager = *static_cast<ComponentManager<C> *>(container.manager);

    assert(component_manager.count() < MAX_COMPONENTS_PER_TYPE && "Component count exceeded");

    if constexpr (has_init<C>)
      has_new_init = true;

    component_manager.push(entity, std::move(component));
    return ComponentReference<C>{ component_manager.get_reference_index(component_manager.count() - 1) };
  }

  template<typename C>
  void remove_component(Entity entity)
  {
    assert(entity != INVALID_ENTITY && "Invalid entity");

    auto &container = component_containers[C::id()];
    assert(container.valid && "Component manager does not exist");

    if (container.remove)
      container.remove(entity);
  }

  template<typename C>
  [[nodiscard]] std::set<C *> get_components(Entity entity)
  {
    assert(created && "Manager not created");
    auto &container = component_containers[C::id()];
    assert(container.valid && "Component manager does not exist");

    auto &component_manager = *static_cast<ComponentManager<C> *>(container.manager);
    std::set<C *> components;
    for (size_t i = 0; i < component_manager.components_count; i++)
    {
      const auto &component = component_manager.components[i];
      if (component.entity == entity)
        components.insert(&component.component);
    }

    return components;
  }

  template<typename C>
  [[nodiscard]] ComponentReference<C> get_component(Entity entity)
  {
    assert(created && "Manager not created");
    auto &container = component_containers[C::id()];
    assert(container.valid && "Component manager does not exist");

    auto &component_manager = *static_cast<ComponentManager<C> *>(container.manager);
    for (size_t i = 0; i < component_manager.count(); i++)
    {
      auto &component = component_manager.get(i);
      if (component.entity == entity)
      {
        const auto reference_index = component_manager.get_reference_index(i);
        return ComponentReference<C>{ reference_index };
      }
    }

    return ComponentReference<C>{ INVALID_INDEX };
  }

  template<typename C>
  [[nodiscard]] ComponentsSpan<C> get_components()
  {
    assert(created && "Manager not created");
    auto &container = component_containers[C::id()];
    assert(container.valid && "Component manager does not exist");

    auto &component_manager = *static_cast<ComponentManager<C> *>(container.manager);
    return ComponentsSpan<C>{ component_manager.components.data(), component_manager.count() };
  }

  inline void set_persistent(Entity entity)
  {
    printf("Setting entity %lu as persistent\n", entity);
    persistent_entities.insert(entity);
  }

  inline void unset_persistent(Entity entity)
  {
    persistent_entities.erase(entity);
  }

  [[nodiscard]] inline bool is_persistent(Entity entity) const
  {
    return persistent_entities.contains(entity);
  }

public:
  std::unordered_map<ComponentType, ComponentManagerContainer> component_containers;

  bool has_new_init{ false };
  std::set<ComponentType> init_components;
  std::set<ComponentType> preupdate_components;
  std::set<ComponentType> update_components;
  std::set<ComponentType> postupdate_components;
  std::set<ComponentType> render_components;
  std::set<ComponentType> collision_components;
  std::set<ComponentType> destroyed_components;

  std::queue<Entity> entity_destroy_queue;
  std::vector<DrawCall> draw_calls;
  std::set<Entity> persistent_entities;

private:
  bool created{ true };
  EntityContainer entity_container;
  size_t last_draw_call_index{ std::numeric_limits<size_t>::max() };

  template<typename C>
  friend ComponentReference<C> add_component(Entity, C &&);

  template<typename C>
  friend void remove_component(Entity);

  template<typename C>
  friend ComponentsSpan<C> get_components();

  template<typename C>
  friend std::set<C *> get_components(Entity);

  template<typename C>
  friend ComponentReference<C> get_component(Entity);

  friend Entity create_entity();

  template<typename C>
  friend Entity add_entity(C &&);

  friend void destroy_entity(Entity);

  friend void destroy_non_persistent_entities();

  friend bool entity_exists(Entity);

  template<typename C>
  friend struct RegisterComponent;

  friend void set_persistent(Entity);
  friend void unset_persistent(Entity);
  friend bool is_persistent(Entity);
};

template<typename C>
inline ComponentReference<C> add_component(Entity entity, C &&component)
{
  return Manager::get().add_component(entity, std::move(component));
}

template<typename C>
inline void remove_component(Entity entity)
{
  Manager::get().remove_component<C>(entity);
}

template<typename C>
[[nodiscard]] inline ComponentsSpan<C> get_components()
{
  return Manager::get().get_components<C>();
}

template<typename C>
[[nodiscard]] inline std::set<C *> get_components(Entity entity)
{
  return Manager::get().get_components<C>(entity);
}

template<typename C>
[[nodiscard]] ComponentReference<C> inline get_component(Entity entity)
{
  return Manager::get().get_component<C>(entity);
}

[[nodiscard]] inline Entity create_entity()
{
  return Manager::get().create_entity();
}

template<typename C>
Entity add_entity(C &&component)
{
  auto &manager = Manager::get();
  auto entity   = manager.create_entity();
  add_component(entity, std::move(component));
  return entity;
}

inline void destroy_entity(Entity entity)
{
  Manager::get().destroy_entity(entity);
}

inline void destroy_non_persistent_entities()
{
  auto &manager = Manager::get();
  for (size_t entity_index = 0; entity_index < manager.entity_container.entities_count; entity_index++)
  {
    const auto entity_id = manager.entity_container.entities[entity_index];
    if (!manager.is_persistent(entity_id))
      manager.destroy_entity(entity_id);
  }

  manager.call_destroy();
}

[[nodiscard]] inline bool entity_exists(Entity entity)
{
  return Manager::get().entity_exists(entity);
}

[[nodiscard]] inline bool is_persistent(Entity entity)
{
  return Manager::get().is_persistent(entity);
}

inline void set_persistent(Entity entity)
{
  Manager::get().set_persistent(entity);
}

inline void unset_persistent(Entity entity)
{
  Manager::get().unset_persistent(entity);
}

template<typename C>
struct ComponentReference
{
  ComponentReference() = default;

  explicit ComponentReference(ReferenceIndex index)
    : index{ index }
  {
  }

  operator bool() const
  {
    return index != INVALID_INDEX;
  }

  [[nodiscard]] inline C &get()
  {
    assert(index != INVALID_INDEX && "Invalid component reference");
    auto &manager              = Manager::get().component_containers[C::id()].template get_manager<C>();
    const auto component_index = manager.get_component_index(index);
    assert(component_index < manager.count());
    return manager.get(component_index);
  }

private:
  ReferenceIndex index{ INVALID_INDEX };
};

template<typename C>
struct RegisterComponent
{
  constexpr inline RegisterComponent()
  {
    auto &manager = Manager::get();
    manager.register_component<C>();
  }

  RegisterComponent(const RegisterComponent &)            = delete;
  RegisterComponent &operator=(const RegisterComponent &) = delete;
  RegisterComponent(RegisterComponent &&)                 = delete;
  RegisterComponent &operator=(RegisterComponent &&)      = delete;
};

#define COMPONENT(C)                         \
  static consteval ComponentType id()        \
  {                                          \
    return hash(#C);                         \
  }                                          \
  Entity entity;                             \
  static inline constexpr const char *name() \
  {                                          \
    return #C;                               \
  }                                          \
  C() = default;

#define EXTERN_COMPONENT_TEMPLATE(C)                                    \
  extern template ComponentReference<C> add_component<C>(Entity, C &&); \
  extern template ComponentReference<C> get_component(Entity);          \
  extern template ComponentsSpan<C> get_components<C>();                \
  extern template Entity add_entity(C &&);                              \
  extern template void remove_component<C>(Entity)

#define COMPONENT_TEMPLATE(C)                                    \
  template ComponentReference<C> add_component<C>(Entity, C &&); \
  template ComponentReference<C> get_component(Entity);          \
  template ComponentsSpan<C> get_components<C>();                \
  template Entity add_entity(C &&);                              \
  template void remove_component<C>(Entity)

#define REGISTER_COMPONENT(C) \
  static inline RegisterComponent<C> register_component_##C {}
