#include "Object.hxx"

namespace rpg {

namespace {
// A few 2-char glyphs coloured so they sit naturally on the tile below.
const ObjectGlyph kTable[OBJECT_COUNT] = {
    {"  ", "38;2;0;0;0"},          // O_NONE
    {"Y\\", "38;2;30;160;50"},     // O_TREE  (slashed trunk)
    {"##", "38;2;60;140;40"},      // O_BUSH
    {",o", "38;2;120;120;130"},    // O_ROCK  (small boulder)
    {"*+", "38;2;230;120;200"},    // O_FLOWER
    {"[]", "38;2;200;170;60"},     // O_CHEST
    {"uu", "38;2;220;140;80"},     // O_MUSHROOM
};
}  // namespace

const ObjectGlyph& objectGlyph(ObjectKind k) { return kTable[k]; }

bool objectBlocks(ObjectKind k) {
  switch (k) {
    case O_TREE:
    case O_BUSH:
    case O_ROCK:
      return true;
    default:
      return false;
  }
}

bool objectPickup(ObjectKind k) {
  switch (k) {
    case O_BUSH:
    case O_FLOWER:
    case O_CHEST:
    case O_MUSHROOM:
      return true;
    default:
      return false;
  }
}

const char* objectName(ObjectKind k) {
  switch (k) {
    case O_TREE:     return "tree";
    case O_BUSH:     return "berry bush";
    case O_ROCK:     return "boulder";
    case O_FLOWER:   return "wildflower";
    case O_CHEST:    return "treasure chest";
    case O_MUSHROOM: return "mushroom";
    default:         return "nothing";
  }
}

}  // namespace rpg
