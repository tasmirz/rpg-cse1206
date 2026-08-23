#include "Chunk.hxx"

#include <cmath>
#include <cstdlib>

namespace rpg {

namespace {
inline int idx(int lx, int ly) { return ly * CHUNK_SIZE + lx; }

// Salt values for the decor hash function. Different objects use different
// salts so their placements don't perfectly correlate.
constexpr uint32_t kDecorSalt = 0xC0FFEE01u;
constexpr uint32_t kChestSalt = 0xC45F00Du;

// Cheap deterministic hash -> [0,1) seeded from world coords + chunk seed.
// Used for sparse decoration placement so we don't need a second Perlin.
inline double hash01(uint32_t seed, int wx, int wy, uint32_t salt) {
  uint32_t h = seed ^ (static_cast<uint32_t>(wx) * 0x9E3779B1u) ^
               (static_cast<uint32_t>(wy) * 0x85EBCA77u) ^ salt;
  h ^= h >> 16;
  h *= 0x7FEB352Du;
  h ^= h >> 15;
  h *= 0x846CA68Bu;
  h ^= h >> 16;
  return (h & 0x00FFFFFFu) / static_cast<double>(0x01000000u);
}
}  // namespace

Chunk::Chunk(int cx, int cy, uint32_t seed)
    : objects_{}, cx_(cx), cy_(cy), seed_(seed) {
  generateOverworld();
  generateCaves();
  placeObjects();
}

void Chunk::generateOverworld() {
  // Caves are layered on top of a Perlin elevation map.
  // Use distinct seeds per layer so changes to one don't shift the others.
  Perlin elevation(seed_ ^ 0xA5A5A5A5u);
  Perlin moisture(seed_ ^ 0x5A5A5A5Au);
  Perlin caveNoise(seed_ ^ 0x3C3C3C3Cu);

  // Sample continuous Perlin at the chunk origin and offset by per-tile
  // fractions of a unit so neighbouring chunks remain continuous.
  double ox = static_cast<double>(cx_) * CHUNK_SIZE;
  double oy = static_cast<double>(cy_) * CHUNK_SIZE;

  for (int ly = 0; ly < CHUNK_SIZE; ly++) {
    for (int lx = 0; lx < CHUNK_SIZE; lx++) {
      double wx = ox + lx;
      double wy = oy + ly;

      double e = elevation.fbm(wx * 0.02, wy * 0.02, 5, 0.5);
      double m = moisture.fbm(wx * 0.03 + 100.0, wy * 0.03 + 100.0, 4, 0.5);

      // e is in roughly [-1, 1]. Biome thresholds:
      //   < -0.30 deep water
      //   <  0.00 water
      //   <  0.05 sand
      //   <  0.45 grass/forest/hills based on moisture & elevation
      //   <  0.70 mountains
      //   >= 0.70 snow
      Tile t;
      if (e < -0.30) {
        t = T_DEEP_WATER;
      } else if (e < 0.00) {
        t = T_WATER;
      } else if (e < 0.05) {
        t = T_SAND;
      } else if (e < 0.45) {
        // Forest where moisture is high, grass otherwise.
        t = (m > 0.10) ? T_FOREST : T_GRASS;
      } else if (e < 0.70) {
        t = T_HILLS;
      } else if (e < 0.85) {
        t = T_MOUNTAIN;
      } else {
        t = T_SNOW;
      }
      tiles_[idx(lx, ly)] = t;

      // Cave mask: 1 if the cave-noise is high enough to seed a cave.
      // We only allow caves underground (elevation below 0.55) so they
      // don't punch through mountains.
      uint8_t cave = 0;
      if (e < 0.55 && caveNoise.sample(wx * 0.08, wy * 0.08) > 0.20) {
        cave = 1;
      }
      caveMask_[idx(lx, ly)] = cave;
    }
  }
}

void Chunk::generateCaves() {
  // Smooth the cave mask with several cellular-automata steps so caves
  // form coherent rooms/tunnels instead of salt-and-pepper noise.
  for (int step = 0; step < 5; step++) {
    cellularAutomataStep(caveMask_.data());
  }

  // Apply caves to the tile grid. Caves replace the surface tile when the
  // mask indicates a wall; otherwise keep the original biome.
  for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++) {
    Tile t = tiles_[i];
    if (t == T_DEEP_WATER || t == T_WATER) continue;  // no caves underwater
    if (caveMask_[i]) {
      tiles_[i] = T_CAVE_WALL;
    }
  }
}

void Chunk::placeObjects() {
  // Sparse deterministic decoration pass — driven by a hash so we don't
  // need a second Perlin grid and neighbouring chunks stay coherent at
  // borders because the hash is keyed on world coordinates.
  for (int ly = 0; ly < CHUNK_SIZE; ly++) {
    for (int lx = 0; lx < CHUNK_SIZE; lx++) {
      Tile t = tiles_[idx(lx, ly)];
      int wx = cx_ * CHUNK_SIZE + lx;
      int wy = cy_ * CHUNK_SIZE + ly;
      double r = hash01(seed_, wx, wy, kDecorSalt);

      ObjectKind obj = O_NONE;
      switch (t) {
        case T_FOREST:
          // ~28% chance of a tree, ~6% chance of a berry bush, ~3% mushroom.
          if (r < 0.28)        obj = O_TREE;
          else if (r < 0.34)   obj = O_BUSH;
          else if (r < 0.37)   obj = O_MUSHROOM;
          break;
        case T_GRASS:
          // ~3% chance of a flower, ~1% mushroom.
          if (r < 0.03)        obj = O_FLOWER;
          else if (r < 0.04)   obj = O_MUSHROOM;
          break;
        case T_SAND:
          // Occasional palm-ish tree on beaches.
          if (r < 0.04)        obj = O_TREE;
          break;
        case T_HILLS:
          if (r < 0.10)        obj = O_ROCK;
          break;
        case T_MOUNTAIN:
          if (r < 0.18)        obj = O_ROCK;
          break;
        default:
          break;
      }
      objects_[idx(lx, ly)] = obj;
    }
  }

  // Chest anchor: at most one chest per chunk, anchored at (7,7) so it's
  // easy to find in the rendered grid. Only replace whatever's there if
  // the cell is grass or sand; never overwrite a tree.
  constexpr int kChestLX = 7;
  constexpr int kChestLY = 7;
  constexpr double kChestChance = 0.5;
  Tile chestTile = tiles_[idx(kChestLX, kChestLY)];
  if ((chestTile == T_GRASS || chestTile == T_SAND) &&
      hash01(seed_, cx_ * CHUNK_SIZE + kChestLX,
             cy_ * CHUNK_SIZE + kChestLY, kChestSalt) < kChestChance &&
      objects_[idx(kChestLX, kChestLY)] == O_NONE) {
    objects_[idx(kChestLX, kChestLY)] = O_CHEST;
  }
}

Tile Chunk::at(int lx, int ly) const {
  if (lx < 0 || lx >= CHUNK_SIZE || ly < 0 || ly >= CHUNK_SIZE) return T_GRASS;
  return tiles_[idx(lx, ly)];
}

ObjectKind Chunk::objectAt(int lx, int ly) const {
  if (lx < 0 || lx >= CHUNK_SIZE || ly < 0 || ly >= CHUNK_SIZE) return O_NONE;
  return objects_[idx(lx, ly)];
}

void Chunk::clearObject(int lx, int ly) {
  if (lx < 0 || lx >= CHUNK_SIZE || ly < 0 || ly >= CHUNK_SIZE) return;
  objects_[idx(lx, ly)] = O_NONE;
}

bool Chunk::hasObject(int lx, int ly) const {
  return objectAt(lx, ly) != O_NONE;
}

void Chunk::cellularAutomataStep(uint8_t* mask) const {
  uint8_t next[CHUNK_SIZE * CHUNK_SIZE];
  for (int ly = 0; ly < CHUNK_SIZE; ly++) {
    for (int lx = 0; lx < CHUNK_SIZE; lx++) {
      int walls = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx == 0 && dy == 0) continue;
          int nx = lx + dx;
          int ny = ly + dy;
          if (nx < 0 || nx >= CHUNK_SIZE || ny < 0 || ny >= CHUNK_SIZE) {
            walls++;  // off-chunk treated as wall to discourage border caves
          } else {
            walls += mask[idx(nx, ny)];
          }
        }
      }
      // Classic 4-5 rule: a wall stays a wall with >=4 neighbours, an
      // empty cell becomes a wall with >=5 neighbours.
      uint8_t cur = mask[idx(lx, ly)];
      next[idx(lx, ly)] = (walls >= 5 || (cur && walls >= 4)) ? 1 : 0;
    }
  }
  for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE; i++) mask[i] = next[i];
}

}  // namespace rpg
