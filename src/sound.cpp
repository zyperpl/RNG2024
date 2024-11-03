#include "sound.hpp"

#include "cached_resource.hpp"

#include <raylib.h>
#include <unordered_map>
#include <vector>

using CachedSound = struct CachedResource<Sound>;

template<>
inline void CachedSound::destruct(CachedSound *res)
{
  UnloadSound(res->data);
  delete res;
}

static std::unordered_map<std::string, std::vector<Sound>> SOUNDS;

GameSound::GameSound(const std::string &file_path)
  : path{ file_path }
{
  Sound raw_sound;

  if (CachedSound::is_used(path))
    raw_sound = CachedSound::use(path);
  else
    raw_sound = CachedSound::add(path, LoadSound(path.c_str()));

  SOUNDS[path].push_back(LoadSoundAlias(raw_sound));
}

GameSound::~GameSound()
{
  CachedSound::free(path);
}

void GameSound::play() const noexcept
{
  for (size_t i = 0; i < SOUNDS[path].size(); ++i)
  {
    const auto &sound = SOUNDS[path][i];
    if (!IsSoundPlaying(sound))
    {
      SetSoundVolume(sound, volume);
      SetSoundPitch(sound, pitch);
      PlaySound(sound);
      last_index = i;
      return;
    }
  }
}

void GameSound::stop() const noexcept
{
  if (last_index < SOUNDS[path].size())
    StopSound(SOUNDS[path][last_index]);
}

bool GameSound::is_playing() const noexcept
{
  if (last_index < SOUNDS[path].size())
    return IsSoundPlaying(SOUNDS[path][last_index]);
  return false;
}
