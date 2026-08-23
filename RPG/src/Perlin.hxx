#pragma once
#include <cstdint>
#include <vector>

namespace rpg {

// Seedable 2D Perlin noise generator.
// Produces smoothly-varying values in roughly [-1, 1].
class Perlin {
 public:
  explicit Perlin(uint32_t seed);

  // Sample noise at continuous coordinates, returned in roughly [-1, 1].
  double sample(double x, double y) const;

  // Fractal Brownian Motion: sum of octaves at decreasing amplitude.
  // octaves >= 1, persistence in (0, 1). Result in roughly [-1, 1].
  double fbm(double x, double y, int octaves, double persistence) const;

 private:
  std::vector<int> p_;  // 512-entry permutation table

  double fade(double t) const;
  double lerp(double a, double b, double t) const;
  double grad(int hash, double x, double y) const;
};

}  // namespace rpg