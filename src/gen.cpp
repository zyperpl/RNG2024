#include "gen.hpp"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

const std::string src_path = "src/";

void generate_entity(const std::string &component_name,
                     const std::string &file_name,
                     const GenerateEntityParams &params)
{
  if (std::filesystem::exists(src_path + file_name + ".hpp") || std::filesystem::exists(src_path + file_name + ".cpp"))
  {
    fprintf(stderr, "Entity %s files already exists\n", file_name.c_str());
    return;
  }

  // hpp
  {
    std::ofstream file(src_path + file_name + ".hpp");
    file << "#pragma once\n\n";

    file << "#include \"manager.hpp\"\n";
    file << "#include \"component.hpp\"\n";
    if (params.has_particles)
      file << "#include \"particles.hpp\"\n";

    if (params.has_sounds)
      file << "#include \"sound.hpp\"\n";

    if (params.is_level_entity)
      file << "#include \"level.hpp\"\n";

    file << "\n";

    file << "struct " << component_name << "\n";
    file << "{\n";
    file << "  COMPONENT(" << component_name << ");\n";
    if (params.is_level_entity)
      file << "  " << component_name << "(const Level::Entity &entity);\n";

    file << "\n";

    file << "  void init();\n";
    if (params.has_preupdate)
      file << "  void preupdate();\n";
    if (params.has_update)
      file << "  void update();\n";
    if (params.has_postupdate)
      file << "  void postupdate();\n";
    if (params.has_render)
      file << "  void render();\n";
    if (params.has_destroyed)
      file << "  void destroyed();\n";
    if (params.has_collision)
      file << "  void collision(Entity other);\n";

    file << "\n";
    file << "private:\n";
    if (params.has_particles)
      file << "  Particle particle;\n";
    if (params.has_sounds)
      file << "  GameSound sound;\n";
    if (params.is_level_entity)
    {
      file << "  int start_x { 0 };\n";
      file << "  int start_y { 0 };\n";
    }

    file << "\n";
    file << "};\n";
    file << "\n";
    file << "EXTERN_COMPONENT_TEMPLATE(" << component_name << ");\n";
  }

  // cpp
  {
    std::ofstream file(src_path + file_name + ".cpp");
    file << "#include \"" << file_name << ".hpp\"\n\n";

    if (params.has_particles)
    {
      file << "#include \"particles.hpp\"\n";
      file << "#include \"game.hpp\"\n";
    }
    if (params.has_sounds)
      file << "#include \"sound.hpp\"\n";
    file << "#include \"utils.hpp\"\n";
    if (params.has_physics)
      file << "#include \"physics.hpp\"\n";
    if (params.has_sprite_renderer || params.has_tile_renderer)
      file << "#include \"renderers.hpp\"\n";

    if (params.is_level_entity)
      file << "#include \"level.hpp\"\n";

    file << "\n";

    file << "REGISTER_COMPONENT(" << component_name << ");\n";
    file << "COMPONENT_TEMPLATE(" << component_name << ");\n";
    if (params.is_level_entity)
      file << "REGISTER_LEVEL_ENTITY(" << component_name << ");\n";

    file << "\n";

    if (params.is_level_entity)
    {
      file << component_name << "::" << component_name << "(const Level::Entity &entity)\n";
      file << "{\n";
      file << "  start_x = entity.position.x + entity.size.w / 2;\n";
      file << "  start_y = entity.position.y + entity.size.h / 2;\n";
      file << "\n";
      file << "}\n";
      file << "\n";
    }

    file << "void " << component_name << "::init()\n";
    file << "{\n";
    if (params.has_physics)
    {
      file << "  auto &physics = add_component(entity, Physics()).get();\n";
      if (params.is_level_entity)
      {
        file << "  physics.x = start_x;\n";
        file << "  physics.y = start_y;\n";
      }
    }

    if (params.has_sprite_renderer)
    {
      file << "  auto &renderer = add_component(entity, SpriteRenderer()).get();\n";
      if (params.is_level_entity)
      {
        file << "  renderer.set_position(start_x, start_y);\n";
      }
    }

    if (params.has_tile_renderer)
    {
      if (params.is_level_entity)
      {
        file << "  add_component(entity, TileRenderer(tileset, tile.position, tile.size, "
                "tile.source_position)).get();\n";
      }
      else
      {
        file << "  auto &renderer = add_component(entity, TileRenderer()).get();\n";
      }
    }

    if (params.has_particles)
      file << "  particle = Game::particle_builder().build();\n";

    if (params.has_sounds)
      file << "  sound = GameSound(\"\");\n";

    file << "\n";

    file << "}\n";

    if (params.has_preupdate)
    {
      file << "void " << component_name << "::preupdate()\n";
      file << "{\n";
      file << "}\n";
      file << "\n";
    }

    if (params.has_update)
    {
      file << "void " << component_name << "::update()\n";
      file << "{\n";
      file << "}\n";
      file << "\n";
    }

    if (params.has_postupdate)
    {
      file << "void " << component_name << "::postupdate()\n";
      file << "{\n";
      file << "}\n";
      file << "\n";
    }

    if (params.has_render)
    {
      file << "void " << component_name << "::render()\n";
      file << "{\n";
      file << "}\n";
      file << "\n";
    }

    if (params.has_destroyed)
    {
      file << "void " << component_name << "::destroyed()\n";
      file << "{\n";
      file << "}\n";
      file << "\n";
    }

    if (params.has_collision)
    {
      file << "void " << component_name << "::collision(Entity other)\n";
      file << "{\n";
      file << "}\n";
      file << "\n";
    }

    file << "\n";
  }

  if (std::filesystem::exists(src_path + "CMakeLists.txt"))
  {
    auto get_last_line = [&](const std::string &match) -> std::string
    {
      std::ifstream file(src_path + "CMakeLists.txt");
      bool found = false;
      std::string line;
      std::string last_line;
      while (std::getline(file, line))
      {
        if (line.find(match) != std::string::npos)
        {
          found = true;
          continue;
        }

        if (found)
        {
          if (line.find(".cpp") != std::string::npos)
            last_line = line;
          else if (line.find(".hpp") != std::string::npos)
            last_line = line;
          else if (line.find(".h") != std::string::npos)
            last_line = line;
          else if (line.find(")") != std::string::npos)
            break;
        }
      }
      return last_line;
    };

    // cpp
    {
      const auto last_line = get_last_line("set(GAME_SOURCES");
      assert(!last_line.empty() && "Could not find GAME_SOURCES");
      if (!last_line.empty() && !last_line.starts_with("  " + file_name + ".cpp"))
      {
        printf("Found last line: \"%s\"\n", last_line.c_str());
        std::string new_line      = "  " + file_name + ".cpp";
        std::string new_last_line = last_line + "\n" + new_line;
        printf("New last line: \"%s\"\n", new_last_line.c_str());

        std::string content;
        {
          std::ifstream file(src_path + "CMakeLists.txt");
          std::string line;
          while (std::getline(file, line))
            content += line + "\n";

          size_t pos = content.find(last_line);
          content.replace(pos, last_line.size(), new_last_line);
        }

        std::ofstream new_file(src_path + "CMakeLists.txt");
        new_file << content;
      }
    }

    // hpp
    {
      const auto last_line = get_last_line("set(GAME_HEADERS");
      assert(!last_line.empty() && "Could not find GAME_HEADERS");
      if (!last_line.empty() && !last_line.starts_with("  " + file_name + ".hpp"))
      {
        printf("Found last line: \"%s\"\n", last_line.c_str());
        std::string new_line      = "  " + file_name + ".hpp";
        std::string new_last_line = last_line + "\n" + new_line;
        printf("New last line: \"%s\"\n", new_last_line.c_str());

        std::string content;
        {
          std::ifstream file(src_path + "CMakeLists.txt");
          std::string line;
          while (std::getline(file, line))
            content += line + "\n";

          size_t pos = content.find(last_line);
          content.replace(pos, last_line.size(), new_last_line);
        }
        std::ofstream new_file(src_path + "CMakeLists.txt");
        new_file << content;
      }
    }
  }
}
