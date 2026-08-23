#pragma once
#include <cstdint>

namespace rpg {

// Decorations / interactables that sit on top of a tile. Stored sparsely
// per-chunk (one byte per cell, O_NONE = empty).
enum ObjectKind : uint8_t {
  O_NONE = 0,
  O_TREE = 1,       // blocks movement, can't pick up — just flavor
  O_BUSH = 2,       // blocks movement, can be chopped for berries (pickup)
  O_ROCK = 3,       // blocks movement on hills/mountains
  O_FLOWER = 4,     // walkable, picked up by 'e'
  O_CHEST = 5,      // walkable, opened by 'e' (one-shot pickup)
  O_MUSHROOM = 6,   // walkable, picked up by 'e'
};

constexpr int OBJECT_COUNT = 7;

struct ObjectGlyph {
  const char* ch;
  const char* fg;
};

// Visual for an object. Background follows the underlying tile.
const ObjectGlyph& objectGlyph(ObjectKind k);

// Whether an object blocks the player from walking onto its tile.
bool objectBlocks(ObjectKind k);

// Whether 'e' should consume/remove the object (a pickup or one-shot).
bool objectPickup(ObjectKind k);

// Short flavor string used by the interaction log.
const char* objectName(ObjectKind k);

}  // namespace rpg
