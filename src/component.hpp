#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

using Entity        = std::uint64_t;
using EntitySet     = std::set<Entity>;
using LevelEntityId = std::string;

constexpr inline Entity INVALID_ENTITY         = 0;
constexpr inline size_t COMPONENT_PADDING_SIZE = 1024;

using ComponentType = std::uint64_t;

consteval inline std::size_t hash(const char *str)
{
  std::size_t h = 5381;
  int c;

  while ((c = *str++))
  {
    h = ((h << 5) + h) + c;
  }

  return h;
}

template<typename C>
struct ComponentPadded
{
  C component;

  constexpr inline ComponentPadded()                  = default;
  ComponentPadded(const ComponentPadded &)            = delete;
  ComponentPadded &operator=(const ComponentPadded &) = delete;
  ComponentPadded(ComponentPadded &&)                 = default;
  ComponentPadded &operator=(ComponentPadded &&)      = default;

  inline ComponentPadded &operator=(C &&component)
  {
    this->component = std::move(component);
    return *this;
  }

private:
  std::byte pad[COMPONENT_PADDING_SIZE > sizeof(C) ? COMPONENT_PADDING_SIZE - sizeof(C) : 1]{};
};

template<typename C>
struct ComponentsSpan
{
  ComponentPadded<C> *const components;
  const size_t count;

  struct Iterator
  {
    using value_type = C;
    using pointer    = C *;
    using reference  = C &;

    ComponentPadded<C> *ptr;

    Iterator &operator++()
    {
      ptr++;
      return *this;
    }

    [[nodiscard]] bool operator!=(const Iterator &other) const
    {
      return ptr != other.ptr;
    }

    [[nodiscard]] C &operator*() const
    {
      return ptr->component;
    }

    [[nodiscard]] C *operator->() const
    {
      return &ptr->component;
    }
  };

  [[nodiscard]] inline bool empty() const noexcept
  {
    return count == 0;
  }

  [[nodiscard]] inline Iterator begin() const noexcept
  {
    return { components };
  }

  [[nodiscard]] inline Iterator end() const noexcept
  {
    return { components + count };
  }

  [[nodiscard]] inline C &front() const noexcept
  {
    assert(count > 0 && "ComponentsSpan is empty");
    return components->component;
  }

  [[nodiscard]] inline C &back() const noexcept
  {
    assert(count > 0 && "ComponentsSpan is empty");
    return (components + count - 1)->component;
  }

  [[nodiscard]] inline C &operator[](size_t index) const noexcept
  {
    assert(index < count && "Index out of bounds");
    return (components + index)->component;
  }
};
