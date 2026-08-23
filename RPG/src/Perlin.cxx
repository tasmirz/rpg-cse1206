#include "Perlin.hxx"

#include <algorithm>
#include <cmath>

namespace rpg {

Perlin::Perlin(uint32_t seed) {
  // Ken Perlin's reference permutation seeded with a simple LCG so the
  // table is reproducible from `seed`.
  p_.resize(256);
  for (int i = 0; i < 256; i++) p_[i] = i;

  uint32_t s = seed ? seed : 1;
  for (int i = 255; i > 0; i--) {
    s = s * 1664525u + 1013904223u;
    int j = static_cast<int>((s >> 16) % static_cast<uint32_t>(i + 1));
    std::swap(p_[i], p_[j]);
  }
  // Duplicate to 512 so we can index without modulo.
  p_.insert(p_.end(), p_.begin(), p_.end());
}

double Perlin::fade(double t) const { return t * t * t * (t * (t * 6 - 15) + 10); }

double Perlin::lerp(double a, double b, double t) const { return a + t * (b - a); }

double Perlin::grad(int hash, double x, double y) const {
  // 8 gradient directions.
  int h = hash & 7;
  double u = h < 4 ? x : y;
  double v = h < 4 ? y : x;
  return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

double Perlin::sample(double x, double y) const {
  int xi = static_cast<int>(std::floor(x)) & 255;
  int yi = static_cast<int>(std::floor(y)) & 255;
  double xf = x - std::floor(x);
  double yf = y - std::floor(y);

  int aa = p_[p_[xi] + yi];
  int ab = p_[p_[xi] + yi + 1];
  int ba = p_[p_[xi + 1] + yi];
  int bb = p_[p_[xi + 1] + yi + 1];

  double u = fade(xf);
  double v = fade(yf);

  double x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
  double x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);
  return lerp(x1, x2, v);
}

double Perlin::fbm(double x, double y, int octaves, double persistence) const {
  double total = 0.0;
  double amplitude = 1.0;
  double frequency = 1.0;
  double max_value = 0.0;
  for (int i = 0; i < octaves; i++) {
    total += sample(x * frequency, y * frequency) * amplitude;
    max_value += amplitude;
    amplitude *= persistence;
    frequency *= 2.0;
  }
  return total / max_value;
}

}  // namespace rpg