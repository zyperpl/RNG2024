#pragma once

#include <memory>
#include <string>
#include <unordered_map>

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

  [[nodiscard]] static inline T use(const std::string &path)
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
