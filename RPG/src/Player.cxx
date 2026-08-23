#include "Player.hxx"

#include "Object.hxx"
#include "Tile.hxx"
#include "World.hxx"

namespace rpg {

Player::MoveResult Player::tryMove(int dx, int dy, const World& world) {
  if (dx == 0 && dy == 0) return MoveResult::NoMove;
  const int nx = x + dx;
  const int ny = y + dy;
  if (!tileWalkable(world.tileAt(nx, ny))) return MoveResult::BlockedTile;
  if (objectBlocks(world.objectAt(nx, ny))) return MoveResult::BlockedObject;
  x = nx;
  y = ny;
  return MoveResult::Moved;
}

}  // namespace rpg
