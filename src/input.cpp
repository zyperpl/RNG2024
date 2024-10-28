#include "input.hpp"

Input Input::instance;

Input &Input::get()
{
  return instance;
}
