#pragma once
#include <cstdint>

namespace rpg {

// Top-down biomes. Each tile renders with one or two terminal characters
// and an ANSI color pair.
enum Tile : uint8_t {
  T_DEEP_WATER = 0,
  T_WATER = 1,
  T_SAND = 2,
  T_GRASS = 3,
  T_FOREST = 4,
  T_HILLS = 5,
  T_MOUNTAIN = 6,
  T_SNOW = 7,
  T_CAVE_FLOOR = 8,  // cellular-automata cave interior
  T_CAVE_WALL = 9,   // cellular-automata cave wall
};

constexpr int TILE_COUNT = 10;

struct Glyph {
  const char* ch;     // 1-2 chars, NUL-terminated
  const char* fg;     // ANSI 24-bit foreground, e.g. "38;2;0;200;0"
  const char* bg;     // ANSI 24-bit background, may be empty
};

const Glyph& glyph(Tile t);

// Whether the player can stand on this tile. Used by both Player's
// collision check and the spawn-finding code so the two stay in sync.
bool tileWalkable(Tile t);

}  // namespace rpg