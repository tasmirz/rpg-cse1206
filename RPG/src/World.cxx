#include "World.hxx"

#include <cstdlib>

namespace rpg {

namespace {
inline int64_t keyOf(int cx, int cy) {
  // Pack (cx, cy) into a single 64-bit key. Bias by a large constant to
  // keep the pair non-negative for typical small worlds.
  return (static_cast<int64_t>(cx + 0x4000) << 32) |
         static_cast<uint32_t>(cy + 0x4000);
}
}  // namespace

World::World(uint32_t seed, int viewRadiusChunks)
    : seed_(seed), radius_(viewRadiusChunks) {}

Tile World::tileAt(int wx, int wy) const {
  int cx = chunkOf(wx);
  int cy = chunkOf(wy);
  auto it = chunks_.find(keyOf(cx, cy));
  if (it == chunks_.end()) return T_GRASS;
  int lx = wx - cx * CHUNK_SIZE;
  int ly = wy - cy * CHUNK_SIZE;
  return it->second->at(lx, ly);
}

Chunk* World::chunkAt(int wx, int wy) const {
  int cx = chunkOf(wx);
  int cy = chunkOf(wy);
  auto it = chunks_.find(keyOf(cx, cy));
  if (it == chunks_.end()) return nullptr;
  return it->second.get();
}

ObjectKind World::objectAt(int wx, int wy) const {
  int cx = chunkOf(wx);
  int cy = chunkOf(wy);
  auto it = chunks_.find(keyOf(cx, cy));
  if (it == chunks_.end()) return O_NONE;
  int lx = wx - cx * CHUNK_SIZE;
  int ly = wy - cy * CHUNK_SIZE;
  return it->second->objectAt(lx, ly);
}

void World::clearObject(int wx, int wy) {
  Chunk* c = chunkAt(wx, wy);
  if (!c) return;
  int lx = wx - chunkOf(wx) * CHUNK_SIZE;
  int ly = wy - chunkOf(wy) * CHUNK_SIZE;
  c->clearObject(lx, ly);
}

void World::ensureChunksAround(int px, int py) {
  int pcx = chunkOf(px);
  int pcy = chunkOf(py);

  // Load any chunks in the active radius that aren't present yet.
  for (int dy = -radius_; dy <= radius_; dy++) {
    for (int dx = -radius_; dx <= radius_; dx++) {
      int64_t k = keyOf(pcx + dx, pcy + dy);
      if (chunks_.find(k) == chunks_.end()) {
        chunks_.emplace(k, std::make_unique<Chunk>(pcx + dx, pcy + dy, seed_));
      }
    }
  }

  // Evict chunks outside radius+1 so the player doesn't notice pop-in but
  // we don't accumulate chunks forever.
  int evict = radius_ + 1;
  for (auto it = chunks_.begin(); it != chunks_.end();) {
    // Decode chunk coords out of the key for eviction distance test.
    int64_t k = it->first;
    int cx = static_cast<int>((k >> 32) - 0x4000);
    int cy = static_cast<int>((k & 0xffffffff) - 0x4000);
    if (std::abs(cx - pcx) > evict || std::abs(cy - pcy) > evict) {
      it = chunks_.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace rpg