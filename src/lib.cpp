#include "lib.hpp"

#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <thread>

#include "utils.hpp"

#if defined(__linux__)
  #include <dlfcn.h>
  #include <fcntl.h>
  #include <unistd.h>
#elif defined(_WIN32)
  #include <windows.h>
#else
  #include "game.hpp"
#endif

static const char *path = "build/src/libgame.so";

bool GameLibrary::is_loaded()
{
#if defined(__linux__)
  if (!std::filesystem::exists(path))
    return false;

  void *new_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL | RTLD_NOLOAD);
  if (new_handle)
  {
    dlclose(new_handle);
    return true;
  }
  return false;
#elif defined(_WIN32)
  if (!std::filesystem::exists(path))
    return false;

  HMODULE new_handle = LoadLibraryA(path);
  if (new_handle)
  {
    FreeLibrary(new_handle);
    return true;
  }
  return false;
#else
  return true;
#endif
}

bool GameLibrary::has_library_changed()
{
  if (!std::filesystem::exists(path))
    return false;

  static std::filesystem::file_time_type last_write_time = std::filesystem::last_write_time(path);
  const auto current_write_time                          = std::filesystem::last_write_time(path);

  const bool is_file_empty = std::filesystem::file_size(path) == 0;

  if (current_write_time != last_write_time && !is_file_empty)
  {
    last_write_time = current_write_time;
    return true;
  }

  return false;
}

bool GameLibrary::is_being_written()
{
  return is_file_written(path);
}

bool GameLibrary::is_valid()
{
  return create_game && update_game && draw_game && destroy_game && reload_game;
}

bool GameLibrary::load()
{
  static uint64_t load_errors           = 0;
  const static uint64_t max_load_errors = 10;

  puts("A");
#if defined(__linux__) || defined(_WIN32)
  if (!std::filesystem::exists(path))
  {
    puts("B");
    printf("Game library does not exist\n");
    return false;
  }
  puts("C");

  if (std::filesystem::file_size(path) < 1024)
  {
    printf("Game library is too small\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return false;
  }

  if (is_being_written())
  {
    printf("Game library is being written\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return false;
  }
#endif

#if defined(__linux__)
  handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
  if (!handle)
  {
    fprintf(stderr, "dlopen failed: %s\n", dlerror());

    if (++load_errors >= max_load_errors)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
      return false;
    }
    else
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      return false;
    }
  }
  dlerror();

  create_game  = reinterpret_cast<create_game_fn>(dlsym(handle, "G_create_game"));
  update_game  = reinterpret_cast<update_game_fn>(dlsym(handle, "G_update_game"));
  draw_game    = reinterpret_cast<draw_game_fn>(dlsym(handle, "G_draw_game"));
  reload_game  = reinterpret_cast<reload_game_fn>(dlsym(handle, "G_reload_game"));
  unload_game  = reinterpret_cast<unload_game_fn>(dlsym(handle, "G_unload_game"));
  destroy_game = reinterpret_cast<destroy_game_fn>(dlsym(handle, "G_destroy_game"));

  if (!create_game || !update_game || !draw_game || !destroy_game || !reload_game || !unload_game)
  {
    fprintf(stderr, "dlsym failed: %s\n", dlerror());

    if (++load_errors >= max_load_errors)
      exit(1);
  }
#elif defined(_WIN32)
  handle = LoadLibraryA(path);
  if (!handle)
  {
    fprintf(stderr, "LoadLibrary failed: %lu\n", GetLastError());

    if (++load_errors >= max_load_errors)
      exit(1);
    else
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return false;
    }
  }

  create_game  = reinterpret_cast<create_game_fn>(GetProcAddress(handle, "G_create_game"));
  update_game  = reinterpret_cast<update_game_fn>(GetProcAddress(handle, "G_update_game"));
  draw_game    = reinterpret_cast<draw_game_fn>(GetProcAddress(handle, "G_draw_game"));
  reload_game  = reinterpret_cast<reload_game_fn>(GetProcAddress(handle, "G_reload_game"));
  unload_game  = reinterpret_cast<unload_game_fn>(GetProcAddress(handle, "G_unload_game"));
  destroy_game = reinterpret_cast<destroy_game_fn>(GetProcAddress(handle, "G_destroy_game"));

  if (!create_game || !update_game || !draw_game || !destroy_game || !reload_game)
  {
    fprintf(stderr, "GetProcAddress failed: %lu\n", GetLastError());

    exit(2);
  }
#else

  this->create_game  = reinterpret_cast<create_game_fn>(G_create_game);
  this->update_game  = reinterpret_cast<update_game_fn>(G_update_game);
  this->draw_game    = reinterpret_cast<draw_game_fn>(G_draw_game);
  this->reload_game  = reinterpret_cast<reload_game_fn>(G_reload_game);
  this->unload_game  = reinterpret_cast<unload_game_fn>(G_unload_game);
  this->destroy_game = reinterpret_cast<destroy_game_fn>(G_destroy_game);

#endif

  load_errors = 0;
  printf("Loaded game library\n");

  return true;
}

bool GameLibrary::unload()
{
  create_game  = nullptr;
  draw_game    = nullptr;
  update_game  = nullptr;
  reload_game  = nullptr;
  unload_game  = nullptr;
  destroy_game = nullptr;

  if (handle)
  {
    printf("Unloading game library\n");
#if defined(__linux__)
    dlclose(handle);
#elif defined(_WIN32)
    FreeLibrary(handle);
#endif
    handle = nullptr;
  }

  assert(!is_loaded());
  return true;
}

bool GameLibrary::reload()
{
  printf("Reloading game library\n");
  if (is_loaded())
  {
    unload();
    printf("Unloaded game library\n");
  }

  assert(!is_loaded());
  printf("Loading game library\n");
  return load();
}
