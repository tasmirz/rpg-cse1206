#include <format>
#include <iostream>
#include <string_view>

#include "header.hxx"

using namespace Graphics;
Color::Color() {}
Color::Color(rgb foreground, rgb background, char flags)
    : foreground(foreground), background(background), flags(flags){};
std::string Color::put() const {
  std::string stash = "\033[";
  for (int i = 0; i < 7; i++)
    if (flags & (1 << i)) stash += std::format("{};", i + 1);
  if (foreground.set)
    stash += std::format("38;2;{};{};{};", (unsigned)foreground.r,
                         (unsigned)foreground.g, (unsigned)foreground.b);
  if (background.set)
    stash += std::format("48;2;{};{};{};", (unsigned)background.r,
                         (unsigned)background.g, (unsigned)background.b);
  stash += "28m";
  // 28 is a hack, 28 shows the char, so it doesnot
  // change any property however it can be put in a
  // string omitting the last char
  return stash;
}

std::string Color::reset() { return "\033[0m"; }
