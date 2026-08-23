#pragma once
#include <array>
#include <cstdint>

#include "Object.hxx"
#include "Perlin.hxx"
#include "Tile.hxx"

namespace rpg {

constexpr int CHUNK_SIZE = 16;  // tiles per side

// A single chunk of the open world. Generated on demand from a shared
// Perlin generator and the chunk's integer (cx, cy) coordinate.
class Chunk {
 public:
  Chunk(int cx, int cy, uint32_t seed);

  Tile at(int lx, int ly) const;            // local 0..CHUNK_SIZE-1
  Tile* raw() { return tiles_.data(); }

  ObjectKind objectAt(int lx, int ly) const;       // O_NONE if empty
  void clearObject(int lx, int ly);                // for pickup
  // True iff there is a non-NONE object at (lx, ly).
  bool hasObject(int lx, int ly) const;

 private:
  void generateOverworld();
  void generateCaves();
  void placeObjects();
  // Cellular automata step: counts cave walls among the 8 neighbours,
  // applying the classic 4-5 rule to the cave mask.
  void cellularAutomataStep(uint8_t* mask) const;

  std::array<Tile, CHUNK_SIZE * CHUNK_SIZE> tiles_{};
  std::array<uint8_t, CHUNK_SIZE * CHUNK_SIZE> caveMask_{};
  std::array<ObjectKind, CHUNK_SIZE * CHUNK_SIZE> objects_{};
  int cx_, cy_;
  uint32_t seed_;
};

}  // namespace rpg
