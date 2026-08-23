#pragma once
#include <cstdint>

namespace rpg {

class World;  // fwd

struct Player {
  int x = 0;
  int y = 0;

  // Try to step by (dx, dy). The world is consulted so we can block
  // water / mountains / walls / solid objects (trees, rocks, bushes).
  // The return value tells the caller why a move was rejected, if any.
  enum class MoveResult {
    Moved,
    BlockedTile,
    BlockedObject,
    NoMove,
  };
  MoveResult tryMove(int dx, int dy, const World& world);
};

}  // namespace rpg
