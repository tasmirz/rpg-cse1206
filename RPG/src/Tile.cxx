#include "Tile.hxx"

namespace rpg {

namespace {
const Glyph kTable[TILE_COUNT] = {
    {"≈≈", "38;2;30;80;180", "48;2;10;30;80"},     // deep water
    {"~~", "38;2;60;140;220", "48;2;20;60;140"},    // water
    {"░░", "38;2;230;210;120", "48;2;90;70;30"},    // sand
    {",,", "38;2;60;180;80", "48;2;30;80;30"},      // grass
    {"TT", "38;2;20;120;40", "48;2;30;80;30"},      // forest (dark green trunks)
    {"∩∩", "38;2;120;100;60", "48;2;60;120;40"},    // hills
    {"▲▲", "38;2;140;140;140", "48;2;90;70;50"},   // mountain
    {"██", "38;2;230;230;240", "48;2;180;180;200"},// snow
    {"··", "38;2;60;50;40", "48;2;20;15;10"},       // cave floor
    {"▓▓", "38;2;90;80;70", "48;2;30;25;20"},      // cave wall
};
}  // namespace

const Glyph& glyph(Tile t) { return kTable[t]; }

bool tileWalkable(Tile t) {
  switch (t) {
    case T_DEEP_WATER:
    case T_WATER:
    case T_MOUNTAIN:
    case T_SNOW:
    case T_CAVE_WALL:
      return false;
    default:
      return true;
  }
}

}  // namespace rpg