#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "Object.hxx"
#include "Player.hxx"
#include "Tile.hxx"
#include "World.hxx"

namespace rpg {

// Top-down terminal renderer. Owns its own previous-frame buffer and
// only emits ANSI escapes for cells that actually changed.
class Renderer {
 public:
  Renderer(int viewW, int viewH);

  // Re-render the viewport centred on the player. Returns the number of
  // cells that had to be updated (for debug / future throttling).
  int render(const World& world, const Player& player);

  // Print a status line below the viewport.
  void status(const std::string& s);

  // Print a second message line (used for interaction feedback).
  void message(const std::string& s);

 private:
  int w_, h_;
  std::vector<Tile> prevTiles_;
  std::vector<ObjectKind> prevObjects_;
  std::vector<bool> prevPlayer_;
};

}  // namespace rpg
