cmake_minimum_required(VERSION 3.14)

add_subdirectory(shaders)

add_subdirectory(external)

include_directories(external)
include_directories(external/raylib/src)
include_directories(external/raylib/src/external/glfw/include)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-ggdb)
  add_compile_options(-O0)
  add_compile_options(-DDEBUG)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  add_compile_options(-ggdb)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3)
endif()

add_compile_options(-Wall)
add_compile_options(-Wno-narrowing)
add_compile_options(-Wno-unused-lambda-capture)
add_compile_options(-Wno-format)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-fdollars-in-identifiers)
  add_compile_options(-Wno-dollar-in-identifier-extension)
endif()

SET(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(ENGINE_SOURCES
  gen.cpp
  input.cpp
  lib.cpp
  main.cpp
  sprite.cpp
)

set(ENGINE_HEADERS
  gen.hpp
  input.hpp
  lib.hpp
  sprite.hpp
)

set(GAME_SOURCES
  block.cpp
  component.cpp
  game.cpp
  level.cpp
  level_loader.cpp
  manager.cpp
  particles.cpp
  physics.cpp
  player.cpp
  renderers.cpp
)

set(GAME_HEADERS
  block.hpp
  component.hpp
  game.hpp
  level.hpp
  level_definitions.hpp
  level_loader.hpp
  mask.hpp
  particles.hpp
  physics.hpp
  player.hpp
  renderers.hpp
  utils.hpp
)

if (EMSCRIPTEN)
  set(ENGINE_HEADERS ${ENGINE_HEADERS} ${GAME_HEADERS})
  set(ENGINE_SOURCES ${ENGINE_SOURCES} ${GAME_SOURCES})
endif()

add_executable(engine ${ENGINE_SOURCES} ${ENGINE_HEADERS})
target_link_libraries(engine PUBLIC raylib)
target_link_options(engine PUBLIC -rdynamic ${SANITIZERS} -fPIC -fno-rtti)
target_precompile_headers(engine PUBLIC
  <raylib.h>
  <raymath.h>
  input.hpp
)
add_dependencies(engine ShaderConversion)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_options(engine PUBLIC -fno-gnu-unique)
endif()

if (NOT EMSCRIPTEN)
  add_library(game SHARED ${GAME_SOURCES} ${GAME_HEADERS})
  add_dependencies(game ShaderConversion)

  target_compile_options(game PUBLIC -fPIC ${SANITIZERS} -fno-plt -fno-rtti)
  target_link_options(game PUBLIC -Wl,--gc-sections)
  target_precompile_headers(game PUBLIC 
    <algorithm>
    <cstdlib>
    <raylib.h>
    <set>
    <vector>
    game.hpp 
    input.hpp
    level.hpp
    level_definitions.hpp
    manager.hpp
    particles.hpp
    physics.hpp
    renderers.hpp 
    sound.hpp
    utils.hpp 
  )

  option(OPTIMIZE_LEVEL_LOADER "Optimize level loader" ON)

  if (OPTIMIZE_LEVEL_LOADER)
    set_source_files_properties(level_loader.cpp PROPERTIES 
      COMPILE_FLAGS "-Oz -s -ffunction-sections -fdata-sections -fno-lto -fno-rtti -fno-inline -fomit-frame-pointer -fmerge-constants -fshort-wchar -fno-stack-check -fpermissive -fno-stack-protector -fno-ident -g0 -UDEBUG -DNDEBUG -U_FORTIFY_SOURCE"
      SKIP_PRECOMPILE_HEADERS ON)

    set_source_files_properties(level.cpp renderers.cpp sprite.cpp cute_aseprite.h PROPERTIES 
      COMPILE_FLAGS "-O3 -s"
      SKIP_PRECOMPILE_HEADERS ON)
  endif()



  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(game PUBLIC -fno-gnu-unique)
  endif()

endif()
