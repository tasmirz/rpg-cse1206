#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "Chunk.hxx"
#include "Object.hxx"

namespace rpg {

// Open-world chunk manager. Generates chunks on demand when the player
// approaches them. Chunks outside the active radius are evicted so the
// world stays at a fixed memory footprint.
class World {
 public:
  explicit World(uint32_t seed, int viewRadiusChunks = 2);

  // World-tile coords (NOT chunk coords). Returns a pointer into a
  // chunk's tile array. The pointer is invalidated on the next call
  // that might evict that chunk, so don't hold it across moves.
  Tile tileAt(int wx, int wy) const;
  Chunk* chunkAt(int wx, int wy) const;

  // Object queries / mutations in world coords.
  ObjectKind objectAt(int wx, int wy) const;
  // Remove the object at (wx,wy) if any. No-op if cell is empty or
  // chunk not loaded.
  void clearObject(int wx, int wy);

  // Make sure all chunks within `viewRadiusChunks` of (px, py) are loaded.
  void ensureChunksAround(int px, int py);

  // Convenience: pixel-ish world coords -> chunk coords.
  static inline int chunkOf(int v) { return v >= 0 ? v / CHUNK_SIZE : -(((-v) + CHUNK_SIZE - 1) / CHUNK_SIZE); }

 private:
  std::unordered_map<int64_t, std::unique_ptr<Chunk>> chunks_;
  uint32_t seed_;
  int radius_;
};

}  // namespace rpg
