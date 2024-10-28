#pragma once

#include <raylib.h>

#include <cstdio>

#define INPUT Input::get()

struct Input
{
  [[nodiscard]] static Input &get();

  struct State
  {
    enum
    {
      PRESSED = 0,
      DOWN    = 2,
      UP      = 3
    } value{ UP };

    constexpr operator bool() const
    {
      return value == DOWN || value == PRESSED;
    }

    constexpr bool pressed() const
    {
      return value == PRESSED;
    }

    constexpr bool down() const
    {
      return value == DOWN;
    }
  };

  State left;
  State right;
  State up;
  State down;
  State action_1;
  State action_2;

  State mouse_left;
  State mouse_right;

  State mouse_wheel_up;
  State mouse_wheel_down;

  State mute;

  void update_discrete()
  {
    auto update_state = [](State &state, const int key)
    {
      if (IsKeyDown(key) && state.value == State::UP)
      {
        state.value = State::PRESSED;
      }
    };

    auto update_mouse_state = [](State &state, const int button)
    {
      if (IsMouseButtonDown(button) && state.value == State::UP)
      {
        state.value = State::PRESSED;
      }
    };

    update_state(left, KEY_LEFT);
    update_state(left, KEY_A);

    update_state(right, KEY_RIGHT);
    update_state(right, KEY_D);

    update_state(up, KEY_UP);
    update_state(up, KEY_W);
    update_state(up, KEY_SPACE);

    update_state(down, KEY_DOWN);
    update_state(down, KEY_S);

    update_state(action_1, KEY_ONE);
    update_state(action_1, KEY_Z);

    update_state(action_2, KEY_TWO);
    update_state(action_2, KEY_X);

    update_state(mute, KEY_M);

    update_mouse_state(mouse_left, MOUSE_LEFT_BUTTON);
    update_mouse_state(mouse_right, MOUSE_RIGHT_BUTTON);

    const auto mouse_wheel_move = GetMouseWheelMove();
    if (mouse_wheel_move < -1e-8)
    {
      mouse_wheel_down.value = State::PRESSED;
      mouse_wheel_up.value   = State::UP;
    }
    else if (mouse_wheel_move > 1e-8)
    {
      mouse_wheel_down.value = State::UP;
      mouse_wheel_up.value   = State::PRESSED;
    }
    else
    {
      if (mouse_wheel_down.value == State::DOWN)
        mouse_wheel_down.value = State::UP;

      if (mouse_wheel_up.value == State::DOWN)
        mouse_wheel_up.value = State::UP;
    }
  }

  template<typename... Keys>
  bool any_key_down(Keys... keys)
  {
    return ((IsKeyDown(keys) || ...));
  };

  template<typename... Keys>
  void update_continuous_state(State &state, Keys... keys)
  {
    if (any_key_down(keys...))
    {
      if (state.value == State::UP)
      {
        state.value = State::PRESSED;
      }
      else
      {
        state.value = State::DOWN;
      }
    }
    else
    {
      state.value = State::UP;
    }
  }

  template<typename... Buttons>
  void update_continous_mouse_state(State &state, Buttons... buttons)
  {
    if ((IsMouseButtonDown(buttons) || ...))
    {
      if (state.value == State::UP)
      {
        state.value = State::PRESSED;
      }
      else
      {
        state.value = State::DOWN;
      }
    }
    else
    {
      state.value = State::UP;
    }
  }

  void update_continuous()
  {
    update_continuous_state(left, KEY_LEFT, KEY_A);
    update_continuous_state(right, KEY_RIGHT, KEY_D);
    update_continuous_state(up, KEY_UP, KEY_W, KEY_SPACE);
    update_continuous_state(down, KEY_DOWN, KEY_S);
    update_continuous_state(action_1, KEY_ONE, KEY_Z);
    update_continuous_state(action_2, KEY_TWO, KEY_X);
    update_continuous_state(mute, KEY_M);

    update_continous_mouse_state(mouse_left, MOUSE_LEFT_BUTTON);
    update_continous_mouse_state(mouse_right, MOUSE_RIGHT_BUTTON);

    const auto mouse_wheel_move = GetMouseWheelMove();
    if (mouse_wheel_move < -1e-8)
    {
      mouse_wheel_down.value = State::DOWN;
      mouse_wheel_up.value   = State::UP;
    }
    else if (mouse_wheel_move > 1e-8)
    {
      mouse_wheel_down.value = State::UP;
      mouse_wheel_up.value   = State::DOWN;
    }
    else
    {
      mouse_wheel_down.value = State::UP;
      mouse_wheel_up.value   = State::UP;
    }
  }

  void update_game_mouse(Rectangle render_rect, Rectangle game_rect)
  {
    const auto mps = GetMousePosition(); // mouse position screenspace

    game_mouse_position.x = (mps.x - render_rect.x) / render_rect.width * game_rect.width;
    game_mouse_position.y = (mps.y - render_rect.y) / render_rect.height * game_rect.height;

#if not defined(EMSCRIPTEN)
    if (should_hide_cursor && IsWindowFocused() && CheckCollisionCircleRec(game_mouse_position, 0, game_rect))
      HideCursor();
    else
      ShowCursor();
#endif
  }

  void update_interface_mouse(Rectangle render_rect, Rectangle interface_rect)
  {
    const auto mps = GetMousePosition();

    interface_mouse_position.x = (mps.x - render_rect.x) / render_rect.width * interface_rect.width;
    interface_mouse_position.y = (mps.y - render_rect.y) / render_rect.height * interface_rect.height;
  }

  [[nodiscard]] Vector2 game_mouse() const
  {
    return game_mouse_position;
  }

  [[nodiscard]] Vector2 interface_mouse() const
  {
    return interface_mouse_position;
  }

  inline void hide_cursor()
  {
    should_hide_cursor = true;
  }

  inline void show_cursor()
  {
    should_hide_cursor = false;
  }

private:
  Input() = default;

  Vector2 game_mouse_position;
  Vector2 interface_mouse_position;
  bool should_hide_cursor{ false };
  static Input instance;
};
